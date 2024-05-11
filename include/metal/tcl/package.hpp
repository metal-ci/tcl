//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef METAL_TCL_PACKAGE_HPP
#define METAL_TCL_PACKAGE_HPP

#include <tcl.h>
#include <exception>
#include <cstdio>

#include <metal/tcl/exception.hpp>

#if defined (TCLOO_VERSION)
#define METAL_TCL_INIT_OO_STUBS() if (Tcl_OOInitStubs(interp) == nullptr) return TCL_ERROR;
#else
#define METAL_TCL_INIT_OO_STUBS()
#endif

#define METAL_TCL_PACKAGE(Package, Version, Name)                                  \
void Package##_init_impl(Tcl_Interp *interp);                                      \
extern "C" int DLLEXPORT Package##_Init(Tcl_Interp *interp)                        \
{                                                                                  \
    if (Tcl_InitStubs(interp, TCL_VERSION, 0) == nullptr)                          \
        return TCL_ERROR;                                                          \
                                                                                   \
    if (Tcl_PkgProvide(interp, #Package, Version) == TCL_ERROR)                    \
        return TCL_ERROR;                                                          \
    try                                                                            \
    {                                                                              \
        Package##_init_impl(interp);                                               \
    }                                                                              \
    catch (...)                                                                    \
    {                                                                              \
        auto obj = metal::tcl::make_exception_object();                            \
        Tcl_SetObjResult(interp, obj.get());                                       \
        return TCL_ERROR;                                                          \
    }                                                                              \
    return TCL_OK;                                                                 \
}                                                                                  \
void Package##_init_impl(Tcl_Interp *Name)


#undef METAL_TCL_INIT_OO_STUBS

#endif //METAL_TCL_PACKAGE_HPP
