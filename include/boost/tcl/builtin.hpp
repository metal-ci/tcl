//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_TCL_BUILTIN_HPP
#define BOOST_TCL_BUILTIN_HPP

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

#include <boost/tcl/builtin/bignum.hpp>
#include <boost/tcl/builtin/bytearray.hpp>
#include <boost/tcl/builtin/dict.hpp>
#include <boost/tcl/builtin/float.hpp>
#include <boost/tcl/builtin/integral.hpp>
#include <boost/tcl/builtin/list.hpp>
 #include <boost/tcl/builtin/proc.hpp>
#include <boost/tcl/builtin/string.hpp>


#endif //BOOST_TCL_BUILTIN_HPP
