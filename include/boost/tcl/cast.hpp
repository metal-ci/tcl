//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_TCL_CAST_HPP
#define BOOST_TCL_CAST_HPP

#include <optional>
#include <boost/tcl/object.hpp>
#include <boost/tcl/interpreter.hpp>
#include <boost/throw_exception.hpp>
#include <boost/utility/string_view.hpp>

namespace boost
{
namespace tcl
{

namespace detail
{

template<std::size_t Idx>
struct rank : rank<Idx - 1> {};

template<>
struct rank<0ull> {};


template<typename T>
struct arg_decay
{
    using type = T;
};

template<typename T>
struct arg_decay<const T>
{
    using type = T;
};

template<typename T>
struct arg_decay<const T &>
{
    using type = T;
};

template<typename T>
struct arg_decay<T &&>
{
    using type = T;
};



template<typename T>
using arg_decay_t = typename arg_decay<T>::type;

template<typename Container>
std::false_type is_map_like_impl(detail::rank<0> r);


template<typename Container>
std::true_type is_map_like_impl(detail::rank<1> r,
                           typename Container::key_type * = nullptr,
                           typename Container::mapped_type* = nullptr);

template<typename Container>
using is_map_like = decltype(is_map_like_impl<Container>(rank<1u>{}));

}


/* The problem with tcl overloads is the ambiguity based on the weak typing\
 *
 * Therefor we have a rank for conversions
 *
 * 1. equal      -> typePtr corresponds to it
 * 2. equivalent -> typePtr is logically convertible (e.g. int -> short)
 * 3. castable   -> can cast, i.e. convert with Tcl (except for string, bc everything is a string)
 * 4. string
 */

struct convert_tag {};

template<typename T>
auto make_object(Tcl_Interp *interp, T && t) -> decltype(tag_invoke(convert_tag{}, interp, std::forward<T>(t)))
{
    return tag_invoke(convert_tag{}, interp, std::forward<T>(t));
}

template<typename T>
struct cast_tag {};


template<typename T>
auto cast(Tcl_Interp * ip, const object_ptr & obj)
    -> decltype(*tag_invoke(cast_tag<detail::arg_decay_t<T>>{}, ip, obj))
{
    auto res = tag_invoke(cast_tag<detail::arg_decay_t<T>>{}, ip, obj);
    if (!res)
        throw_exception(std::bad_cast());
    return *std::move(res);
}

template<typename T>
struct equal_type_tag{};

template<typename T>
struct equivalent_type_tag{};


namespace detail
{

template<typename T>
auto is_equal_type_impl(Tcl_Interp * interp, const Tcl_Obj * obj,detail::rank<2>)
    -> decltype(tag_invoke(equal_type_tag<detail::arg_decay_t<T>>{}, interp, obj))
{
    return tag_invoke(equal_type_tag<detail::arg_decay_t<T>>{}, interp, obj);
}

template<typename T>
auto is_equal_type_impl(Tcl_Interp * interp, const Tcl_Obj * obj, detail::rank<1>)
    -> decltype(tag_invoke(equal_type_tag<detail::arg_decay_t<T>>{}, *obj->typePtr))
{
    if (obj->typePtr == nullptr)
        return false;
    return tag_invoke(equal_type_tag<detail::arg_decay_t<T>>{}, *obj->typePtr);
}

template<typename T>
auto is_equal_type_impl(Tcl_Interp * interp, const Tcl_Obj * type, detail::rank<0>)
{
    return false;
}


template<typename T>
auto is_equivalent_type_impl(const Tcl_ObjType * type, detail::rank<1>)
    -> decltype(tag_invoke(equivalent_type_tag<detail::arg_decay_t<T>>{}, *type))
{
    if (type == nullptr)
        return false;
    return tag_invoke(equivalent_type_tag<detail::arg_decay_t<T>>{}, *type);
}

template<typename T>
auto is_equivalent_type_impl(const Tcl_ObjType * type, detail::rank<0>)
{
    return false;
}


}

template<typename T>
bool is_equal_type(Tcl_Interp * interp, const Tcl_Obj * obj)
{
    return detail::is_equal_type_impl<T>(interp, obj, detail::rank<2>{});
}


template<typename T>
bool is_equivalent_type(const Tcl_ObjType * type)
{
    return detail::is_equivalent_type_impl<T>(type, detail::rank<1>{});
}

template<typename T>
auto try_cast(Tcl_Interp * ip, Tcl_Obj * obj)
    -> decltype(tag_invoke(cast_tag<detail::arg_decay_t<T>>{}, ip, obj))
{
    return tag_invoke(cast_tag<detail::arg_decay_t<T>>{}, ip, obj);
}


template<typename T>
auto try_cast_without_implicit_string(Tcl_Interp * ip, Tcl_Obj * obj)
    -> decltype(try_cast<T>(ip, obj))
{
    constexpr bool is_string_like = std::is_convertible_v<T, string_view> ||
                                    std::is_constructible_v<T, const char*, std::size_t>;
    if (is_string_like &&
        (obj->typePtr && obj->typePtr->name && obj->typePtr->name != string_view("string")))
        return {};
    return try_cast<T>(ip, obj);
}

template<typename T>
inline std::optional<std::optional<T>>  tag_invoke(
    cast_tag<std::optional<T>>,
    Tcl_Interp * interp,
    boost::intrusive_ptr<Tcl_Obj> val)
{
  if (!val || (val->typePtr == nullptr && val->length == 0))
    return std::nullopt;
  return cast<T>(cast_tag<T>{}, interp, std::move(val));
}



template<typename T>
inline object_ptr tag_invoke(
    const convert_tag & tag,
    Tcl_Interp * interp,
    const std::optional<T> & val)
{
  if (val)
    return tag_invoke(tag, interp, *val);
  else
    return Tcl_NewObj();
}

}
}

#endif //BOOST_TCL_CAST_HPP
