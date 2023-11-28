//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <tclbind/namespace.hpp>
#include <tclbind/eval.hpp>

#include "doctest.h"

extern Tcl_Interp *interp;


TEST_CASE("namespace")
{
  auto ns = tclbind::create_namespace<int>(interp, "test-namespace", 42);
  CHECK(tclbind::get_if<double>(ns) == nullptr);
  CHECK(tclbind::get_if<int>(ns) != nullptr);
  CHECK(tclbind::get<int>(*ns) == 42);
  CHECK_THROWS(tclbind::get<double>(*ns));
  Tcl_DeleteNamespace(ns);
}
