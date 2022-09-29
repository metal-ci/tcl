//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_TCL_OBJECT_HPP
#define BOOST_TCL_OBJECT_HPP

#include <tcl.h>
#include <optional>
#include <boost/intrusive_ptr.hpp>


inline void intrusive_ptr_add_ref(Tcl_Obj * obj) { Tcl_IncrRefCount(obj); }
inline void intrusive_ptr_release(Tcl_Obj * obj) { Tcl_DecrRefCount(obj); }

namespace boost::tcl
{

using object_ptr  = boost::intrusive_ptr<Tcl_Obj>;

template<typename T> struct cast_tag;
template<typename T> struct equal_type_tag;

inline Tcl_Obj ** tag_invoke(
        const cast_tag<Tcl_Obj *> &,
        Tcl_Interp *,
        Tcl_Obj* &val)
{
    return &val;
}

inline std::optional<object_ptr> tag_invoke(
        const cast_tag<object_ptr> &,
        Tcl_Interp *,
        object_ptr val)
{
    return val;
}

inline object_ptr tag_invoke(
        const struct convert_tag &,
        Tcl_Interp*,
        object_ptr ptr)
{
    return ptr;
}

template<typename E>
inline bool tag_invoke(
        const equal_type_tag<E> &,
        const Tcl_ObjType &)
{
    return true;
}


}


#endif //BOOST_TCL_OBJECT_HPP
