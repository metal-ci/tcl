//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "boost/tcl/builtin/proc.hpp"
#include <boost/tcl/builtin.hpp>
#include <boost/tcl/package.hpp>
#include <boost/tcl/command.hpp>
#include <boost/tcl/class.hpp>
#include <boost/describe.hpp>


BOOST_TCL_PACKAGE(Proc, "1.0", mod)
{
  boost::tcl::create_command(mod, "call-callback")
    .add_function_with_interp(+[](Tcl_Interp * ip, boost::tcl::proc cmd, int x, int y )
    {
      return cmd(x, y);
    });
}