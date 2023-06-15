//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_TCL_BUILTIN_INTEGRAL_HPP
#define BOOST_TCL_BUILTIN_INTEGRAL_HPP

#include <boost/tcl/cast.hpp>
#include <boost/multiprecision/gmp.hpp>
#include <concepts>

namespace boost::multiprecision
{

template <>
struct number_category<mp_int> : public std::integral_constant<int, number_kind_integer>
{};

}

namespace boost::tcl
{

inline std::optional<bool> tag_invoke(
        cast_tag<bool>,
        Tcl_Interp * interp,
        boost::intrusive_ptr<Tcl_Obj> val)
{
    int res = 0;
    if (TCL_OK == Tcl_GetBooleanFromObj(interp, val.get(), &res))
        return res != 0;
    else
        return std::nullopt;
}

template<typename Bool>
inline object_ptr tag_invoke(
        const struct convert_tag &,
        Tcl_Interp*,
        Bool && b,
        std::enable_if_t<std::is_same_v<Bool, bool>> * = nullptr)
{
    return Tcl_NewBooleanObj(b);
}


inline std::optional<int> tag_invoke(
        cast_tag<int>,
        Tcl_Interp * interp,
        boost::intrusive_ptr<Tcl_Obj> val)
{
    int res = 0;
    if (TCL_OK == Tcl_GetIntFromObj(interp, val.get(), &res))
        return res;
    else
        return std::nullopt;
}

inline object_ptr tag_invoke(const struct convert_tag &, Tcl_Interp*, int i)
{
    return Tcl_NewIntObj(i);
}

inline std::optional<unsigned int> tag_invoke(
        cast_tag<unsigned int>,
        Tcl_Interp * interp,
        boost::intrusive_ptr<Tcl_Obj> val)
{
    unsigned int res = 0;
    if (TCL_OK == Tcl_GetIntFromObj(interp, val.get(), reinterpret_cast<int*>(&res)))
        return res;
    else
        return std::nullopt;
}

inline object_ptr tag_invoke(const convert_tag &, Tcl_Interp*, unsigned int i)
{
    return Tcl_NewIntObj(i);
}


inline std::optional<Tcl_WideInt> tag_invoke(
        cast_tag<Tcl_WideInt>,
        Tcl_Interp * interp,
        boost::intrusive_ptr<Tcl_Obj> val)
{
    Tcl_WideInt res = 0;
    if (TCL_OK == Tcl_GetWideIntFromObj(interp, val.get(), &res))
        return res;
    else
        return std::nullopt;
}

inline object_ptr tag_invoke(const struct convert_tag &, Tcl_Interp*, Tcl_WideInt i)
{
    return Tcl_NewWideIntObj(i);
}

inline std::optional<Tcl_WideUInt> tag_invoke(
        cast_tag<Tcl_WideUInt>,
        Tcl_Interp * interp,
        boost::intrusive_ptr<Tcl_Obj> val)
{
    Tcl_WideUInt res = 0;
    if (TCL_OK == Tcl_GetWideIntFromObj(interp, val.get(), reinterpret_cast<Tcl_WideInt *>(&res)))
        return res;
    else
        return std::nullopt;
}

template<typename I>
inline bool tag_invoke(
        const equivalent_type_tag<I> & tag,
        const Tcl_ObjType & type,
        std::enable_if_t<std::is_integral_v<I> && !std::is_enum_v<I>, I> * = nullptr)
{
    if (type.name)
        return  type.name == string_view("bignum")
             || type.name == string_view("int");
    return false;
}


inline bool tag_invoke(
        const equal_type_tag<Tcl_WideInt> & tag,
        const Tcl_ObjType & type)
{
    return type.name && type.name == string_view("int");
}

inline object_ptr tag_invoke(const struct convert_tag &, Tcl_Interp*, Tcl_WideUInt i)
{
    return Tcl_NewWideIntObj(i);
}

template<typename I, typename = std::enable_if_t<std::is_integral_v<I>>>
inline std::optional<I> tag_invoke(
        cast_tag<I>,
        Tcl_Interp * interp,
        boost::intrusive_ptr<Tcl_Obj> val)
{
    using tag = cast_tag<std::conditional_t<std::is_signed_v<I>, signed int, unsigned int>>;
    return tag_invoke(tag{}, interp, std::move(val));
}

template<typename I>
inline object_ptr tag_invoke(
        const convert_tag & tag,
        Tcl_Interp* interp,
        I i,
        std::enable_if_t<std::is_integral_v<I> && !std::is_enum_v<I> && !std::is_same_v<I, bool>, I> * = nullptr)
{
    using type = std::conditional_t<std::is_signed_v<I>, signed int, unsigned int>;
    return tag_invoke(tag, interp, static_cast<type>(i));
}


inline bool tag_invoke(
        const equal_type_tag<bool> & tag,
        const Tcl_ObjType & type)
{
    return type.name && type.name == string_view("boolean");
}


}

#endif //BOOST_TCL_BUILTIN_INTEGRAL_HPP
