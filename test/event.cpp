//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <metal/tcl/event.hpp>
#include <metal/tcl/eval.hpp>

#include "doctest.h"

extern Tcl_Interp *interp;

struct test_event_source : metal::tcl::event_source
{
  bool fired = false;
  void setup(int flags)
  {
    if (!fired)
    {
      Tcl_Time block_time = {0, 0};
      block_time.sec = 1;
      Tcl_SetMaxBlockTime(&block_time);
    }
  }
  void check(int flags)
  {
    fired = true;

    fprintf(stderr, "check(%d)\n", flags);
    Tcl_QueueEvent(
        new metal::tcl::event(
            [](int flags)
            {
              Tcl_SetVar(interp, "foobar", "done", 0);
              return true;
            }),
        TCL_QUEUE_HEAD);
  }
};


TEST_CASE("event")
{
  test_event_source te;
  CHECK_NOTHROW(metal::tcl::eval(interp, "vwait foobar"));
}
