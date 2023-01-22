//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_TCL_CLASS_HPP
#define BOOST_TCL_CLASS_HPP

#include <tcl.h>
#include <typeinfo>
#include <cstring>

#include <boost/describe/class.hpp>
#include <boost/describe/bases.hpp>
#include <boost/describe/enumerators.hpp>

#include <boost/mp11/algorithm.hpp>

#include <boost/tcl/cast.hpp>

namespace boost::tcl
{
struct sub_command;
inline struct command & create_command(Tcl_Interp *interp, const char * name);

namespace detail
{

template<typename T>
void make_class_description(Tcl_Obj* obj)
{
    const auto nm = typeid(T).name();
    const auto nlen = std::strlen(nm);
    const auto len = nlen + sizeof("boost::tcl::class::0x") + sizeof(void*)*2;
    obj->bytes = Tcl_Alloc(len);
    obj->length = snprintf(obj->bytes, len, "boost::tcl::class::%s::%p", nm, obj->internalRep.twoPtrValue.ptr1);
}

struct class_commands
{
    Tcl_Interp * interp;
    Tcl_Command cmd;

    ~class_commands()
    {
        printf("XYZ %p %p\n", interp, cmd);
        //Tcl_DeleteCommandFromToken(interp, cmd);
    }

    template<typename T>
    class_commands * dup(T* ptr, const char * name)
    {
        printf("DUP %p %p\n", interp, cmd);

        if (make)
            return make(ptr, interp, name);
        else
            return nullptr;
    }

    class_commands * (*make)(void*, Tcl_Interp *, const char *);
};

template<typename T>
class_commands * make_class_command(T*, Tcl_Interp * interp, const char * name);

template<typename T>
void make_class_duplicate(Tcl_Obj * src,
                          std::enable_if_t<!std::is_copy_constructible_v<T>, Tcl_Obj> * dup = nullptr);

template<typename T>
void make_class_duplicate(Tcl_Obj * src,
                          std::enable_if_t<std::is_copy_constructible_v<T>, Tcl_Obj> * dup = nullptr);

template<typename T>
constexpr Tcl_DupInternalRepProc * duplicate_class =
        std::is_copy_constructible_v<T> ? &make_class_duplicate<T> : nullptr;

template<typename T>
void delete_class(Tcl_Obj * obj)
{
    delete static_cast<T*>(obj->internalRep.twoPtrValue.ptr1);
    if (obj->internalRep.twoPtrValue.ptr2 != nullptr)
        delete static_cast<class_commands*>(obj->internalRep.twoPtrValue.ptr2);

}

template<typename T>
void delete_class_reference(Tcl_Obj * obj)
{
    if (obj->internalRep.twoPtrValue.ptr2 != nullptr)
        delete static_cast<class_commands*>(obj->internalRep.twoPtrValue.ptr2);
}

template<typename Target, typename Actual>
void * describe_cast_impl(const std::type_info & ti, Actual * pp)
{
    if (ti == typeid(Target))
        return static_cast<Target*>(pp);

    void * v = nullptr;
    mp11::mp_for_each<describe::describe_bases<Target, describe::mod_public>>(
            [&](auto m)
            {
                using type = typename decltype(m)::type;
                auto * p_ = describe_cast_impl<type>(ti, pp);
                if (p_)
                    v = p_;
            });
    return v;
}

template<typename Actual>
void * describe_cast(const std::type_info & ti, void * pp)
{
    return describe_cast_impl<Actual>(ti, static_cast<Actual*>(pp));
}


template<typename Target>
bool describe_can_cast_impl(const std::type_info & ti)
{
    if (ti == typeid(Target))
        return true;

    bool v = false;
    mp11::mp_for_each<describe::describe_bases<Target, describe::mod_public>>(
            [&](auto m)
            {
                using type = typename decltype(m)::type;
                v |= describe_can_cast_impl<type>(ti);
            });
    return v;
}


template<typename Actual>
bool describe_can_cast(const std::type_info & ti)
{
    return describe_can_cast_impl<Actual>(ti);
}


template<typename Target>
Target invoke_describe_cast(void * actual,
                            void*(*impl)(const std::type_info &, void*))
{
    return static_cast<Target>(impl(typeid(std::remove_pointer_t<Target>), actual));
}

template<typename Target>
bool invoke_describe_can_cast(bool(*impl)(const std::type_info &))
{
    return impl(typeid(std::remove_pointer_t<Target>));
}

inline void make_reference_duplicate(Tcl_Obj * src,
                              Tcl_Obj * dup)
{
    dup->internalRep.twoPtrValue.ptr1 = src->internalRep.twoPtrValue.ptr1;
    dup->typePtr = src->typePtr;
    dup->internalRep.twoPtrValue.ptr2 =
            static_cast<class_commands*>(src->internalRep.twoPtrValue.ptr2)->dup(dup->internalRep.twoPtrValue.ptr1, Tcl_GetString(dup));
}


}

struct extended_type_info : Tcl_ObjType
{
    void*(*cast_impl)(const std::type_info &, void*);
    bool (*can_cast_impl)(const std::type_info &);

    template<typename Target>
    Target* cast(void * from) const
    {
        return detail::invoke_describe_cast<Target*>(from, cast_impl);
    }

    template<typename Target>
    bool can_cast() const
    {
        return detail::invoke_describe_can_cast<Target>(can_cast_impl);
    }
};

template<typename T>
inline const extended_type_info class_type {
    {
        .name = typeid(T).name(),
        .freeIntRepProc = detail::delete_class<T>,
        .dupIntRepProc = detail::duplicate_class<T>,
        .updateStringProc = &detail::make_class_description<T>
    },
    .cast_impl = &detail::describe_cast<T>,
    .can_cast_impl = &detail::describe_can_cast<T>
};

template<typename T>
inline const extended_type_info class_reference_type {
    {
        .name = typeid(T).name(),
        .freeIntRepProc = detail::delete_class_reference<T>,
        .dupIntRepProc = &detail::make_reference_duplicate,
        .updateStringProc = &detail::make_class_description<T>
    },
    .cast_impl = &detail::describe_cast<T>,
    .can_cast_impl = &detail::describe_can_cast<T>
};

namespace detail
{

template<typename T>
void make_class_duplicate(Tcl_Obj * src,
                          std::enable_if_t<std::is_copy_constructible_v<T>, Tcl_Obj> * dup)
{
    auto src_ = static_cast<T*>(src->internalRep.twoPtrValue.ptr1);
    auto ptr = new T(*src_);
    dup->internalRep.twoPtrValue.ptr1 = ptr;
    dup->typePtr = src->typePtr;
    Tcl_GetString(dup);

    dup->internalRep.twoPtrValue.ptr2 = static_cast<class_commands*>(src->internalRep.twoPtrValue.ptr2)->dup(
            ptr, Tcl_GetString(dup));
}

}

template<typename T>
inline std::decay_t<T> * tag_invoke(
        cast_tag<T>,
        Tcl_Interp * interp,
        boost::intrusive_ptr<Tcl_Obj> val,
        decltype(describe::describe_bases<std::decay_t<T>, describe::mod_public>())  * = nullptr)
{
    if (val->typePtr == nullptr)
        return nullptr;
    using type = std::decay_t<T>;
    if (val->typePtr == &class_type<type>)
        return static_cast<type*>(val->internalRep.twoPtrValue.ptr1);

    if (!string_view(val->typePtr->name).starts_with("<boost.tcl.class["))
        return nullptr;

    const auto p = static_cast<const extended_type_info*>(val->typePtr);
    return p->cast<std::decay_t<T>>(val->internalRep.twoPtrValue.ptr1);
}

template<typename T>
inline object_ptr tag_invoke(
        const struct convert_tag &,
        Tcl_Interp* interp,
        T && value,
        decltype(boost::describe::describe_bases<std::decay_t<T>, describe::mod_public>())  * type = nullptr)
{
    object_ptr obj = Tcl_NewObj();
    Tcl_InvalidateStringRep(obj.get());
    obj->typePtr = &class_type<std::decay_t<T>>;
    auto ptr = new std::decay_t<T>(std::forward<T>(value));
    obj->internalRep.twoPtrValue.ptr1 = ptr;
    Tcl_GetString(obj.get());
    obj->internalRep.twoPtrValue.ptr2 = detail::make_class_command(ptr, interp, Tcl_GetString(obj.get()));
    return obj;
}

template<typename T>
inline object_ptr tag_invoke(
        const struct convert_tag &,
        Tcl_Interp* interp,
        T & value,
        decltype(boost::describe::describe_bases<std::decay_t<T>, describe::mod_public>())  * type = nullptr)
{
    object_ptr obj = Tcl_NewObj();
    Tcl_InvalidateStringRep(obj.get());
    obj->typePtr = &class_type<T>;
    auto ptr = &value;
    obj->internalRep.twoPtrValue.ptr1 = ptr;
    Tcl_GetString(obj.get());
    obj->internalRep.twoPtrValue.ptr2 = detail::make_class_command(ptr, interp, Tcl_GetString(obj.get()));
    return obj;
}

template<typename T>
inline object_ptr tag_invoke(
        const struct convert_tag &,
        Tcl_Interp* interp,
        T * value,
        decltype(boost::describe::describe_bases<T, describe::mod_public>())  * type = nullptr)
{
    object_ptr obj = Tcl_NewObj();
    Tcl_InvalidateStringRep(obj.get());
    obj->typePtr = &class_type<T>;
    auto ptr = value;
    obj->internalRep.twoPtrValue.ptr1 = ptr;
    obj->internalRep.twoPtrValue.ptr2 = detail::make_class_command(ptr, interp, Tcl_GetString(obj.get()));
    return obj;
}


template<typename E>
inline bool tag_invoke(
        const equivalent_type_tag<E> & tag,
        const Tcl_ObjType & type,
        decltype(boost::describe::describe_bases<E, describe::mod_public>())  * = nullptr)
{
    if (!string_view(type.name).starts_with("<boost.tcl.class["))
        return false;

    const auto p = static_cast<const extended_type_info*>(&class_type<E>);
    return p->can_cast<E>();
}



template<typename E>
inline bool tag_invoke(
        const equal_type_tag<E> & tag,
        const Tcl_ObjType & type,
        decltype(boost::describe::describe_bases<E, describe::mod_public>())  * = nullptr)
{
    return & type == &class_type<E>;
}



}

#endif //BOOST_TCL_CLASS_HPP
