//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_TCL_PACKAGE_HPP
#define BOOST_TCL_PACKAGE_HPP

#include <tcl.h>
#include <exception>
#include <cstdio>

#include <boost/tcl/exception.hpp>

#define BOOST_TCL_PACKAGE(Pacakage, Version, Name)                                  \
void Pacakage##_init_impl(Tcl_Interp *interp);                                      \
extern "C" int DLLEXPORT Pacakage##_Init(Tcl_Interp *interp)                        \
{                                                                                   \
    if (Tcl_InitStubs(interp, TCL_VERSION, 0) == NULL)                              \
        return TCL_ERROR;                                                           \
                                                                                    \
    if (Tcl_PkgProvide(interp, #Pacakage, Version) == TCL_ERROR)                    \
        return TCL_ERROR;                                                           \
    try                                                                             \
    {                                                                               \
        Pacakage##_init_impl(interp);                                               \
    }                                                                               \
    catch (...)                                                                     \
    {                                                                               \
        auto obj = boost::tcl::make_exception_object();                             \
        Tcl_SetObjResult(interp, obj.get());                                        \
        return TCL_ERROR;                                                           \
    }                                                                               \
    return TCL_OK;                                                                  \
}                                                                                   \
void Pacakage##_init_impl(Tcl_Interp *Name)


namespace boost
{
namespace tcl
{



}
}

#endif //BOOST_TCL_PACKAGE_HPP
