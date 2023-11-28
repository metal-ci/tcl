//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef TCLBIND_LIST_HPP
#define TCLBIND_LIST_HPP

#include <tclbind/cast.hpp>
#include <boost/core/span.hpp>

namespace tclbind
{

inline std::optional<boost::span<Tcl_Obj*>> tag_invoke(
        cast_tag<boost::span<Tcl_Obj*>>,
        Tcl_Interp * interp,
        boost::intrusive_ptr<Tcl_Obj> val)
{
    int sz;
    Tcl_Obj ** res;
    if (TCL_OK != Tcl_ListObjGetElements(interp, val.get(), &sz, &res))
        return std::nullopt;

    return boost::span<Tcl_Obj*>(res, sz);
}

inline object_ptr tag_invoke(const struct convert_tag &, Tcl_Interp*, boost::span<Tcl_Obj*> sv)
{
    return Tcl_NewListObj(sv.size(), sv.data());
}

inline bool tag_invoke(
        const equal_type_tag<boost::span<Tcl_Obj*>> & tag,
        const Tcl_ObjType & type)
{
    return type.name && type.name == boost::core::string_view("list");
}

// range conversions

template<typename Container>
inline std::optional<Container> tag_invoke(
        cast_tag<Container>,
        Tcl_Interp * interp,
        boost::intrusive_ptr<Tcl_Obj> val,
        std::enable_if_t<
                std::is_constructible_v<Container, typename Container::value_type *, typename Container::value_type * >
            && !std::is_same_v<typename Container::value_type, char>
            && !std::is_same_v<typename Container::value_type, unsigned char>
            && !std::is_same_v<typename Container::value_type, signed char>
            && !detail::is_map_like<Container>::value> * = nullptr,
        decltype(std::begin(std::declval<Container>()))* = nullptr,
        decltype(std::end  (std::declval<Container>()))* = nullptr,
        decltype(tag_invoke(cast_tag<typename Container::value_type>{}, interp, val))* = nullptr)
{
    int sz;
    Tcl_Obj ** res;
    if (TCL_OK != Tcl_ListObjGetElements(interp, val.get(), &sz, &res))
        return std::nullopt;

    using type = typename Container::value_type;
    std::vector<type> vec;
    vec.reserve(sz);

    for (auto s : boost::span<Tcl_Obj*>(res, sz))
    {
        auto v = try_cast<type>(interp, s);
        if (v)
            vec.push_back(std::move(*v));
        else
            return std::nullopt;
    }

    return Container(vec.begin(), vec.end());
}

template<typename Container>
inline object_ptr tag_invoke(
        const struct convert_tag &,
        Tcl_Interp* interp,
        const Container & c,
        std::enable_if_t<
                 !std::is_same_v<typename Container::value_type, char>
              && !std::is_same_v<typename Container::value_type, unsigned char>
              && !std::is_same_v<typename Container::value_type, signed char>> * = nullptr,
        decltype(std::begin(std::declval<Container>()))* = nullptr,
        decltype(std::end  (std::declval<Container>()))* = nullptr,
        decltype(make_object(interp, std::declval<typename Container::value_type>()))* = nullptr)
{
    std::vector<Tcl_Obj *> res ;
    res.reserve(std::size(c));
    for (const auto & v : c)
        res.emplace_back(make_object(interp, v).detach());

    return Tcl_NewListObj(res.size(), res.data());
}

template<typename Container>
inline bool tag_invoke(
        const equivalent_type_tag<Container> & tag,
        const Tcl_ObjType & type,
        std::enable_if_t<
                ! detail::is_map_like<Container>::value
                && !std::is_same_v<typename Container::value_type, char>
                && !std::is_same_v<typename Container::value_type, unsigned char>
                && !std::is_same_v<typename Container::value_type, signed char>> * = nullptr,
        decltype(std::begin(std::declval<Container>()))* = nullptr,
        decltype(std::end  (std::declval<Container>()))* = nullptr)
{
    return type.name && type.name == boost::core::string_view("list");
}

}

#endif //TCLBIND_LIST_HPP
