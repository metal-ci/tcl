//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <metal/tcl/allocator.hpp>
#include <string>

#include "doctest.h"

TEST_SUITE_BEGIN("allocator");

TEST_CASE("alloc")
{
    std::basic_string<char, std::char_traits<char>, metal::tcl::allocator<char>> str;

    for (auto i = 0; i < 1000; i++)
        str += "xyz ";

}

TEST_SUITE_END();