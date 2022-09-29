//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_TCL_ENUM_HPP
#define BOOST_TCL_ENUM_HPP

#include <boost/describe/enum.hpp>
#include <boost/describe/enumerators.hpp>

#include <boost/mp11/algorithm.hpp>

namespace boost::tcl
{

template<typename E>
inline const Tcl_ObjType enum_type =
    {
        .name = typeid(E).name(),
        .dupIntRepProc =
            +[](Tcl_Obj * src, Tcl_Obj * dup)
            {
                dup->internalRep.wideValue = src->internalRep.wideValue;
                dup->typePtr = src->typePtr;
            },
        .updateStringProc =
            +[](Tcl_Obj * obj)
            {
                auto e = static_cast<E>(obj->internalRep.wideValue);
                boost::mp11::mp_for_each<boost::describe::describe_enumerators<E>>(
                        [&](auto desc)
                        {
                            if (desc.value == e)
                            {
                                auto len = std::strlen(desc.name) + 1;
                                obj->bytes = Tcl_Alloc(len);
                                obj->length =  len - 1;
                                std::copy_n(desc.name, len, obj->bytes);
                            }
                        });
            },
        .setFromAnyProc = nullptr
    };

template<typename E>
inline std::optional<E> tag_invoke(
        cast_tag<E>,
        Tcl_Interp * interp,
        boost::intrusive_ptr<Tcl_Obj> val,
        decltype(boost::describe::describe_enumerators<E>())  * type = nullptr)
{
    if (val->typePtr == &enum_type<E>)
        return static_cast<E>(val->internalRep.wideValue);

    int len = 0;
    std::string_view str{Tcl_GetStringFromObj(val.get(), & len), static_cast<std::size_t>(len)};

    std::optional<E> res;

    boost::mp11::mp_for_each<boost::describe::describe_enumerators<E>>(
            [&](auto desc )
            {
                if (desc.name == str)
                    res = desc.value;
            });

    return res;
}

template<typename E>
inline object_ptr tag_invoke(
        const struct convert_tag &,
        Tcl_Interp*,
        E enm,
        decltype(boost::describe::describe_enumerators<E>())  * type = nullptr)
{
    object_ptr obj = Tcl_NewObj();
    Tcl_InvalidateStringRep(obj.get());
    obj->typePtr = &enum_type<E>;
    obj->internalRep.wideValue = static_cast<Tcl_WideInt>(enm);
    Tcl_GetString(obj.get());
    return obj;
}

template<typename E>
inline bool tag_invoke(
        const equal_type_tag<E> & tag,
        const Tcl_ObjType & type,
        decltype(boost::describe::describe_enumerators<E>())  * = nullptr)
{
    return & type == &enum_type<E>;
}



}

#endif //BOOST_TCL_ENUM_HPP
