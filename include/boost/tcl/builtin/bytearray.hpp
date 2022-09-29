//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_TCL_BUILTIN_BYTEARRAY_HPP
#define BOOST_TCL_BUILTIN_BYTEARRAY_HPP

#include <boost/tcl/cast.hpp>
#include <boost/core/span.hpp>

namespace boost::tcl
{

inline std::optional<span<unsigned char>> tag_invoke(
        cast_tag<span<unsigned char>>,
        Tcl_Interp *,
        boost::intrusive_ptr<Tcl_Obj> val)
{
    int sz;
    auto * c = Tcl_GetByteArrayFromObj(val.get(), &sz);
    return span<unsigned char>(c, sz);
}

inline object_ptr tag_invoke(const struct convert_tag &, Tcl_Interp*, span<unsigned char> sv)
{
    return Tcl_NewByteArrayObj(sv.data(), sv.size());
}

inline bool tag_invoke(
        const equal_type_tag<span<unsigned char>> & tag,
        const Tcl_ObjType & type)
{
    return type.name && type.name == string_view("bytearray");
}

template<typename Allocator>
inline bool tag_invoke(
        const equal_type_tag<std::vector<unsigned char, Allocator>> & tag,
        const Tcl_ObjType & type)
{
    return type.name && type.name == string_view("bytearray");
}

}

#endif //BOOST_TCL_BUILTIN_BYTEARRAY_HPP
