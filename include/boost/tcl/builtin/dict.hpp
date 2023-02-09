//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_TCL_BUILTIN_DICT_HPP
#define BOOST_TCL_BUILTIN_DICT_HPP

#include <boost/tcl/cast.hpp>
#include <boost/core/span.hpp>

namespace boost::tcl
{

// map conversions

template<typename Container>
inline std::optional<Container> tag_invoke(
        cast_tag<Container>,
        Tcl_Interp * interp,
        boost::intrusive_ptr<Tcl_Obj> val,
        std::enable_if_t<
            detail::is_map_like<Container>::value &&
            std::is_convertible_v<decltype(std::declval<Container>()[std::declval<typename Container::key_type>()]),
                                typename Container::mapped_type>> * = nullptr,
        decltype(tag_invoke(cast_tag<typename Container::mapped_type>{}, interp, val))* = nullptr,
        decltype(tag_invoke(cast_tag<typename Container::key_type>{}, interp, val))* = nullptr)
{
    Container  res;
    if (val->length == 0)
      return res;
    Tcl_Obj *key, *value;
    Tcl_DictSearch search;
    int done;
    if (TCL_OK != Tcl_DictObjFirst(interp, val.get(), &search,  &key, &value, &done))
        return std::nullopt;

    bool failed = false;
    do
    {
        auto key_ = try_cast<typename Container::key_type>(interp, key);
        Tcl_Obj *to;
        if (TCL_OK != Tcl_DictObjGet(interp, val.get(), key, &to))
        {
            failed = true;
            break;
        }

        auto val_ = try_cast<typename Container::mapped_type>(interp, to);
        if (key_ && val_)
            res[*key_] = *val_;
        else
        {
            failed = true;
            break;
        }
        Tcl_DictObjNext(&search, &key, &value, &done);
    }
    while (!done);

    Tcl_DictObjDone(&search);
    if (failed)
        return std::nullopt;
    return res;
}

template<typename Container>
inline object_ptr tag_invoke(
        const struct convert_tag &,
        Tcl_Interp * interp,
        const Container & c,
        std::enable_if_t<detail::is_map_like<Container>::value> * = nullptr,
        decltype(make_object(interp, std::declval<typename Container::mapped_type>()))* = nullptr,
        decltype(make_object(interp, std::declval<typename Container::key_type>()))* = nullptr)
{
    object_ptr res = Tcl_NewDictObj();
    for (const auto & [k, v] : c)
        Tcl_DictObjPut(interp, res.get(), make_object(interp, k).detach(), make_object(interp, v).detach());

    return res;
}

template<typename Container>
inline bool tag_invoke(
        const equivalent_type_tag<Container> & tag,
        const Tcl_ObjType & type,
        std::enable_if_t<
                detail::is_map_like<Container>::value &&
                std::is_convertible_v<decltype(std::declval<Container>()[std::declval<typename Container::key_type>()]),
                        typename Container::mapped_type>> * = nullptr)
{
    return type.name && type.name == string_view("dict");
}

}

#endif //BOOST_TCL_BUILTIN_DICT_HPP
