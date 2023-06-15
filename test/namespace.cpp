//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/tcl/namespace.hpp>
#include <boost/tcl/eval.hpp>

#include "doctest.h"

extern Tcl_Interp *interp;


TEST_CASE("namespace")
{
  auto ns = boost::tcl::create_namespace<int>(interp, "test-namespace", 42);
  CHECK(boost::tcl::get_if<double>(ns) == nullptr);
  CHECK(boost::tcl::get_if<int>(ns) != nullptr);
  CHECK(boost::tcl::get<int>(*ns) == 42);
  CHECK_THROWS(boost::tcl::get<double>(*ns));
  Tcl_DeleteNamespace(ns);
}
