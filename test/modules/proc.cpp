//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "metal/tcl/builtin/proc.hpp"
#include <metal/tcl/builtin.hpp>
#include <metal/tcl/package.hpp>
#include <metal/tcl/command.hpp>
#include <metal/tcl/class.hpp>
#include <boost/describe.hpp>


METAL_TCL_PACKAGE(Proc, "1.0", mod)
{
  metal::tcl::create_command(mod, "call-callback")
    .add_function_with_interp(+[](Tcl_Interp * ip, metal::tcl::proc cmd, int x, int y )
    {
      return cmd(x, y);
    });
}