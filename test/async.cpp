//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <tclbind/async.hpp>
#include <tclbind/eval.hpp>

#include "doctest.h"

extern Tcl_Interp *interp;

struct test_async : tclbind::async
{
  bool called = false;
  int invoke(Tcl_Interp * interp, int code) override
  {
    called = true;
    return TCL_OK;
  }
};


TEST_CASE("async")
{
  test_async ta;
  CHECK(!ta.called);
  CHECK_NOTHROW(tclbind::eval(interp, "puts nothing").value());
  CHECK(!ta.called);
  ta.mark();
  CHECK(!ta.called);
  CHECK_NOTHROW(tclbind::eval(interp, "puts something").value());
  CHECK(ta.called);
}
