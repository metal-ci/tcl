//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_TCL_BUILTIN_FLOAT_HPP
#define BOOST_TCL_BUILTIN_FLOAT_HPP

#include <boost/tcl/cast.hpp>
#include <boost/utility/string_view.hpp>

namespace boost::tcl
{

inline std::optional<double> tag_invoke(
        cast_tag<double>,
        Tcl_Interp * interp,
        boost::intrusive_ptr<Tcl_Obj> val)
{
    double res = 0;
    if (TCL_OK == Tcl_GetDoubleFromObj(interp, val.get(), &res))
        return res;
    else
        return std::nullopt;
}

inline object_ptr tag_invoke(const struct convert_tag &,  Tcl_Interp*, double i)
{
    return Tcl_NewDoubleObj(i);
}

inline std::optional<float>  tag_invoke(
        cast_tag<float>,
        Tcl_Interp * interp,
        boost::intrusive_ptr<Tcl_Obj> val)
{
    using tag = cast_tag<double>;
    return tag_invoke(tag{}, interp, std::move(val));
}

inline object_ptr tag_invoke(
        const convert_tag & tag,
        Tcl_Interp interp,
        float i)
{
    return tag_invoke(tag, interp, static_cast<double>(i));
}

inline bool tag_invoke(
        const equal_type_tag<double> & tag,
        const Tcl_ObjType & type)
{
    return type.name && type.name == string_view("double");
}

template<typename F>
inline bool tag_invoke(
        const equivalent_type_tag<F> & tag,
        const Tcl_ObjType & type,
        std::enable_if_t<std::is_floating_point_v<F>, F> * = nullptr)
{
    if (type.name)
        return type.name == string_view("bignum")
            || type.name == string_view("int")
            || type.name == string_view("double");
    return false;
}


}

#endif //BOOST_TCL_BUILTIN_FLOAT_HPP
