//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <metal/tcl/namespace.hpp>
#include <metal/tcl/eval.hpp>

#include "doctest.h"

extern Tcl_Interp *interp;


TEST_CASE("namespace")
{
  auto ns = metal::tcl::create_namespace<int>(interp, "test-namespace", 42);
  CHECK(metal::tcl::get_if<double>(ns) == nullptr);
  CHECK(metal::tcl::get_if<int>(ns) != nullptr);
  CHECK(metal::tcl::get<int>(*ns) == 42);
  CHECK_THROWS(metal::tcl::get<double>(*ns));
  Tcl_DeleteNamespace(ns);
}
