//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef METAL_TCL_BUILTIN_STRING_HPP
#define METAL_TCL_BUILTIN_STRING_HPP

#include <metal/tcl/cast.hpp>
#include <boost/core/detail/string_view.hpp>

#include <string>

namespace metal::tcl
{

inline std::optional<boost::core::string_view> tag_invoke(
    cast_tag<boost::core::string_view>,
    Tcl_Interp *,
    boost::intrusive_ptr<Tcl_Obj> val)
{
  int sz;
  char * c = Tcl_GetStringFromObj(val.get(), &sz);
  return boost::core::string_view(c, sz);
}

inline object_ptr tag_invoke(const struct convert_tag &, Tcl_Interp*, boost::core::string_view sv)
{
  return Tcl_NewStringObj(sv.data(), sv.size());
}


inline bool tag_invoke(
    const equal_type_tag<boost::core::string_view> & tag,
    const Tcl_ObjType & type)
{
  return type.name && type.name == boost::core::string_view("string");
}

template<typename Traits, typename Allocator>
inline bool tag_invoke(
        const equal_type_tag<std::basic_string<char, Traits, Allocator>> & tag,
        const Tcl_ObjType & type)
{
    return type.name && type.name == boost::core::string_view("string");
}

template<typename Traits>
inline bool tag_invoke(
        const equal_type_tag<boost::basic_string_view<char, Traits>> & tag,
        const Tcl_ObjType & type)
{
    return type.name && type.name == boost::core::string_view("string");
}

template<typename StringLike>
inline bool tag_invoke(
        const equivalent_type_tag<StringLike> & tag,
        const Tcl_ObjType & type,
        std::enable_if_t<
                std::is_convertible_v<StringLike, boost::core::string_view> ||
                std::is_constructible_v<StringLike, const char*, std::size_t>, StringLike> * = nullptr)
{
    return type.name &&
           type.name == boost::core::string_view("string");
}

template<typename StringLike>
inline std::optional<StringLike> tag_invoke(
        cast_tag<StringLike>,
        Tcl_Interp *,
        boost::intrusive_ptr<Tcl_Obj> val,
        std::enable_if_t<
            std::is_convertible_v<StringLike, boost::core::string_view> ||
            std::is_constructible_v<StringLike, const char*, std::size_t>, StringLike> * = nullptr)
{
    std::optional<StringLike> res;
    int sz;
    char * c = Tcl_GetStringFromObj(val.get(), &sz);

    res.emplace(c, sz);
    return std::move(res);
}

}

#endif //METAL_TCL_BUILTIN_STRING_HPP
