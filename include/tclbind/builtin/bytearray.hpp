//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef TCLBIND_BUILTIN_BYTEARRAY_HPP
#define TCLBIND_BUILTIN_BYTEARRAY_HPP

#include <tclbind/cast.hpp>
#include <boost/core/span.hpp>

namespace tclbind
{

inline std::optional<boost::span<unsigned char>> tag_invoke(
        cast_tag<boost::span<unsigned char>>,
        Tcl_Interp *,
        boost::intrusive_ptr<Tcl_Obj> val)
{
    int sz;
    auto * c = Tcl_GetByteArrayFromObj(val.get(), &sz);
    return boost::span<unsigned char>(c, sz);
}

inline object_ptr tag_invoke(const struct convert_tag &, Tcl_Interp*, boost::span<unsigned char> sv)
{
    return Tcl_NewByteArrayObj(sv.data(), sv.size());
}

inline bool tag_invoke(
        const equal_type_tag<boost::span<unsigned char>> & tag,
        const Tcl_ObjType & type)
{
    return type.name && type.name == boost::core::string_view("bytearray");
}

template<typename Allocator>
inline bool tag_invoke(
        const equal_type_tag<std::vector<unsigned char, Allocator>> & tag,
        const Tcl_ObjType & type)
{
    return type.name && type.name == boost::core::string_view("bytearray");
}

}

#endif //TCLBIND_BUILTIN_BYTEARRAY_HPP
