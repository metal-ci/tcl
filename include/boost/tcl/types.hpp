//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_TCL_TYPES_HPP
#define BOOST_TCL_TYPES_HPP

#include <tcl.h>
#include <boost/utility/string_view.hpp>
#include <boost/multiprecision/gmp.hpp>
#include <tclTomMath.h>

namespace boost
{
namespace tcl
{

struct get_value {};
struct set_value {};
struct new_value {};


Tcl_Obj * tag_invoke(new_value, int value)
{
    return Tcl_NewIntObj(value);
}

void tag_invoke(set_value, Tcl_Obj * obj, int value)
{
    Tcl_SetIntObj(obj, value);
}

bool tag_invoke(get_value, Tcl_Interp * interp, Tcl_Obj * obj, int & value)
{
    return Tcl_GetIntFromObj(interp, obj,  &value) == TCL_OK;
}

Tcl_Obj * tag_invoke(new_value, long value)
{
    return Tcl_NewLongObj(value);
}

void tag_invoke(set_value, Tcl_Obj * obj, long value)
{
    Tcl_SetLongObj(obj, value);
}

bool tag_invoke(get_value, Tcl_Interp * interp, Tcl_Obj * obj, long & value)
{
    return Tcl_GetLongFromObj(interp, obj,  &value) == TCL_OK;
}

Tcl_Obj * tag_invoke(new_value, double value)
{
    return Tcl_NewDoubleObj(value);
}

void tag_invoke(set_value, Tcl_Obj * obj, double value)
{
    Tcl_SetDoubleObj(obj, value);
}

bool tag_invoke(get_value, Tcl_Interp * interp, Tcl_Obj * obj, double & value)
{
    return Tcl_GetDoubleFromObj(interp, obj,  &value) == TCL_OK;
}

Tcl_Obj * tag_invoke(new_value, bool value)
{
    return Tcl_NewBooleanObj(value);
}

void tag_invoke(set_value, Tcl_Obj * obj, bool value)
{
    Tcl_SetBooleanObj(obj, value);
}

bool tag_invoke(get_value, Tcl_Interp * interp, Tcl_Obj * obj, bool & value)
{
    int val = 0;
    auto res = Tcl_GetBooleanFromObj(interp, obj,  &val) == TCL_OK;
    value = val != 0;
    return res;
}

Tcl_Obj * tag_invoke(new_value, long long value)
{
    return Tcl_NewWideIntObj(value);
}

void tag_invoke(set_value, Tcl_Obj * obj, long long value)
{
    Tcl_SetWideIntObj(obj, value);
}

bool tag_invoke(get_value, Tcl_Interp * interp, Tcl_Obj * obj, long long & value)
{
    return Tcl_GetWideIntFromObj(interp, obj,  &value) == TCL_OK;
}


/*
Tcl_Obj * tag_invoke(new_value, bool value)
{
    TclBNInitBignumFromLong()
}

void tag_invoke(set_value, Tcl_Obj * obj, bool value)
{
    Tcl_SetBignumFromObj(obj, value);
}

bool tag_invoke(get_value, Tcl_Interp * interp, Tcl_Obj * obj, bool & value)
{
    return Tcl_GetBignumFromObj()
}
*/

}
}

#endif //BOOST_TCL_TYPES_HPP
