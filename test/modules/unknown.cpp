//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/tcl/builtin.hpp>
#include <boost/tcl/package.hpp>
#include <boost/tcl/string_command.hpp>
#include <boost/tcl/builtin/bignum.hpp>


BOOST_TCL_PACKAGE(Unknown, "1.0", mod)
{
  boost::tcl::create_string_command(
      mod, "unknown",
      [](int argc, const char * argv[])
      {
          using std::operator""sv;
          assert(argv[0] == "::unknown"sv);

          if (argv[1] == "foo"sv)
          {
            assert(argc == 3);
            assert(argv[2] == "123"sv);
          }
          else if (argv[1] == "bar"sv)
          {
            assert(argc == 4);
            assert(argv[2] == "12"sv);
            assert(argv[3] == "3.4"sv);
          }
          else
            assert(!"Invalid command");
      });
}