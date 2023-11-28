//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include <tclbind/expr.hpp>
#include <tclbind/builtin/integral.hpp>
#include <tclbind/builtin/string.hpp>

#include "doctest.h"

using namespace boost;

extern Tcl_Interp *interp;
namespace tcl = tclbind;

TEST_CASE("expr")
{
  CHECK(tcl::expr<int>(interp, 42) == 42);
  CHECK(tcl::expr<int>(interp, "23 + 12") == 35);
  CHECK_THROWS(tcl::expr<int>(interp, "12 / 0").value());
}