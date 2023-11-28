//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "tclbind/builtin/proc.hpp"
#include <tclbind/builtin.hpp>
#include <tclbind/package.hpp>
#include <tclbind/command.hpp>
#include <tclbind/class.hpp>
#include <boost/describe.hpp>


TCLBIND_PACKAGE(Proc, "1.0", mod)
{
  tclbind::create_command(mod, "call-callback")
    .add_function_with_interp(+[](Tcl_Interp * ip, tclbind::proc cmd, int x, int y )
    {
      return cmd(x, y);
    });
}