//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <tclbind/var.hpp>
#include <tclbind/eval.hpp>
#include <tclbind/builtin/integral.hpp>
#include <boost/asio.hpp>

#include "doctest.h"

using namespace boost;

extern Tcl_Interp *interp;

namespace tcl = tclbind;

TEST_CASE("var")
{
  CHECK(!tcl::unset(interp, "foo"));
  CHECK(!tcl::try_get<int>(interp, "foo"));

  CHECK_THROWS(tcl::eval(interp, "puts $foo"));
  tcl::set(interp, "foo" , "42");
  CHECK_NOTHROW(tcl::eval(interp, "puts $foo"));
  CHECK(tcl::get<int>(interp, "foo") == 42);
  tcl::append(interp, "foo" , "13");
  CHECK_NOTHROW(tcl::eval(interp, "puts $foo"));
  CHECK(tcl::get<int>(interp, "foo") == 4213);
  CHECK(tcl::unset(interp, "foo"));
  CHECK(!tcl::try_get(interp, "foo").has_value());
  CHECK_THROWS(tcl::eval(interp, "puts $foo"));
  CHECK(!tcl::try_get(interp, "foo").has_value());
}


TEST_CASE("var-array")
{
  CHECK(!tcl::unset(interp, "foo_arr", "bar"));
  CHECK(tcl::try_get<int>(interp, "foo_arr", "bar") == 123);

  CHECK_THROWS(tcl::eval(interp, "puts $foo_arr"));
  tcl::set(interp, "foo_arr", "bar" , "42");
  CHECK_NOTHROW(tcl::eval(interp, "puts $foo_arr(bar)"));
  CHECK(tcl::get<int>(interp, "foo_arr", "bar") == 42);
  tcl::append(interp, "foo_arr", "bar" , "13");
  CHECK_NOTHROW(tcl::eval(interp, "puts $foo_arr(bar)"));
  CHECK(tcl::get<int>(interp, "foo_arr", "bar") == 4213);
  CHECK(tcl::unset(interp, "foo_arr", "bar"));
  CHECK(!tcl::try_get(interp, "foo_arr", "bar").has_value());
  CHECK_THROWS(tcl::eval(interp, "puts $foo_arr(bar)"));
  CHECK(!tcl::try_get(interp, "foo_arr", "bar").has_value());
}



TEST_CASE("var-trace")
{
  bool called = false;
  tcl::tracer tr(
      [&](Tcl_Interp * ip, const char * name, const char * key, int flags)
      {
        CHECK(name == std::string_view("foo"));
        CHECK(key  == std::string_view("x"));
        CHECK(flags == TCL_TRACE_WRITES);
        called = true;
      }, interp, "foo", TCL_TRACE_WRITES);

  CHECK(called == false);
  tcl::eval(interp, "set foo(x) 42");
  CHECK(called == true);

}
