//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef METAL_TCL_BUILTIN_HPP
#define METAL_TCL_BUILTIN_HPP

/*
 * The conversions aren't super easy to do, since everything is a string.
 * This would make overloading rather inconvenient, if we ever use a string.
 *
 * Therefore, we go a three-fold order here
 *
 * 1. IS type
 * 2. can be converted with tag_invoke
 * 3. string
 *
 * That is, if the type matches we can call it (without
 *
 */

#include <metal/tcl/builtin/bytearray.hpp>
#include <metal/tcl/builtin/dict.hpp>
#include <metal/tcl/builtin/float.hpp>
#include <metal/tcl/builtin/integral.hpp>
#include <metal/tcl/builtin/list.hpp>
#include <metal/tcl/builtin/proc.hpp>
#include <metal/tcl/builtin/string.hpp>


#endif //METAL_TCL_BUILTIN_HPP
