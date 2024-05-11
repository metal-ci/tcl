//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include <metal/tcl/builtin/bignum.hpp>

#include "doctest.h"

TEST_CASE("bignum")
{
    metal::tcl::bignum mi{mp_digit(42)};
    auto s = mi.str(1000, std::ios_base::fmtflags(0));
}