//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef TCLBIND_BUILTIN_HPP
#define TCLBIND_BUILTIN_HPP

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

#include <tclbind/builtin/bytearray.hpp>
#include <tclbind/builtin/dict.hpp>
#include <tclbind/builtin/float.hpp>
#include <tclbind/builtin/integral.hpp>
#include <tclbind/builtin/list.hpp>
#include <tclbind/builtin/proc.hpp>
#include <tclbind/builtin/string.hpp>


#endif //TCLBIND_BUILTIN_HPP
