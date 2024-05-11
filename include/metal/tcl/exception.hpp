//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef TCL_EXCEPTION_HPP
#define TCL_EXCEPTION_HPP

#include <boost/throw_exception.hpp>
#include <metal/tcl/cast.hpp>
#include <metal/tcl/interpreter.hpp>
#include <metal/tcl/object.hpp>

#include <boost/system/result.hpp>
#include <boost/utility/string_view.hpp>

#include <tcl.h>

#include <exception>
#include <new>
#include <cstring>
#include <optional>


namespace metal::tcl
{

/// Exception for an error that has no equivalent in C++
struct tcl_exception : std::exception
{
    tcl_exception(boost::intrusive_ptr<Tcl_Obj> obj)
        :  obj_(std::move(obj))
    {

    }

    const char * what() const noexcept override
    {
        return Tcl_GetString(obj_.get());
    }

    const boost::intrusive_ptr<Tcl_Obj> object() const
    {
        return obj_;
    }

  private:

    boost::intrusive_ptr<Tcl_Obj> obj_;
};

inline const Tcl_ObjType exception_ptr_type =
    {
        .name = "std::exception_ptr",
        .freeIntRepProc =
            +[](Tcl_Obj * obj)
            {
                static_assert(sizeof(std::exception_ptr) == sizeof(void*));
                std::exception_ptr ptr;
                *reinterpret_cast<void**>(&ptr) = obj->internalRep.twoPtrValue.ptr1;
            },
        .dupIntRepProc =
            +[](Tcl_Obj * src, Tcl_Obj * dup)
            {
                static_assert(sizeof(std::exception_ptr) == sizeof(void*));
                auto p = new (&dup->internalRep.twoPtrValue.ptr1) std::exception_ptr(
                        *reinterpret_cast<std::exception_ptr*>(&src->internalRep.twoPtrValue.ptr1));
                dup->typePtr = src->typePtr;
            },
        .updateStringProc =
            +[](Tcl_Obj * obj)
            {
                try
                {
                    static_assert(sizeof(std::exception_ptr) == sizeof(void*));
                    std::rethrow_exception(*reinterpret_cast<std::exception_ptr*>(&obj->internalRep.twoPtrValue.ptr1));
                }
                catch (std::exception & e)
                {
                    auto msg = e.what();
                    auto len = std::strlen(msg) + 1;
                    obj->bytes = Tcl_Alloc(len);
                    obj->length = len-1;
                    std::copy_n(e.what(), len, obj->bytes);
                }
                catch(...)
                {
                    static char msg[] = "unknown exception_type";
                    auto len = std::strlen(msg) + 1;
                    obj->bytes = Tcl_Alloc(len);
                    obj->length = len-1;
                    std::copy_n(msg, len, obj->bytes);
                }
            },
        .setFromAnyProc = nullptr
    };


inline object_ptr make_exception_object(
        std::exception_ptr ex = std::current_exception())
{
    boost::intrusive_ptr<Tcl_Obj> obj = Tcl_NewObj();
    Tcl_InvalidateStringRep(obj.get());
    obj->typePtr = &exception_ptr_type;
    new (&obj->internalRep.twoPtrValue.ptr1) std::exception_ptr(ex);
    Tcl_GetString(obj.get());
    return obj;
}

BOOST_NORETURN inline void throw_result(Tcl_Interp * interp)
{
    Tcl_RegisterObjType(&exception_ptr_type);
    auto res = Tcl_GetObjResult(interp);
    if (res->typePtr == &exception_ptr_type)
        std::rethrow_exception(*reinterpret_cast<std::exception_ptr*>(&res->internalRep.twoPtrValue.ptr1));
    else
        boost::throw_exception(tcl_exception(res));
}

BOOST_NORETURN inline void throw_result(const interpreter_ptr & interp)
{
  throw_result(interp.get());
}

template<typename T>
using result = boost::system::result<T, object_ptr>;

inline std::optional<std::exception_ptr> tag_invoke(
        cast_tag<std::exception_ptr>,
        Tcl_Interp *,
        boost::intrusive_ptr<Tcl_Obj> val)
{
    std::exception_ptr res;

    if (val->typePtr == &exception_ptr_type)
        return *reinterpret_cast<std::exception_ptr*>(&val->internalRep.twoPtrValue.ptr1);
    else if (!val->typePtr || val->typePtr->name == boost::core::string_view("string"))
        return std::make_exception_ptr(tcl_exception(val));
    return std::nullopt;
}

inline object_ptr tag_invoke(const struct convert_tag &,  Tcl_Interp*, std::exception_ptr ep)
{
    return make_exception_object(ep);
}

inline bool tag_invoke(
        const equal_type_tag<std::exception_ptr> & tag,
        const Tcl_ObjType & type)
{
    return & type == &exception_ptr_type;
}
}

namespace boost
{

BOOST_NORETURN
inline
void throw_exception_from_error( const boost::intrusive_ptr<Tcl_Obj> & e,
                                 boost::source_location const & loc )
{
  boost::throw_exception(metal::tcl::tcl_exception(e), loc);
}

}


#endif //TCL_EXCEPTION_HPP