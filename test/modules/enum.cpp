//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/tcl/builtin.hpp>
#include <boost/tcl/package.hpp>
#include <boost/tcl/command.hpp>
#include <boost/tcl/enum.hpp>


enum foobar { foo = 1, bar };
BOOST_DESCRIBE_ENUM(foobar, foo, bar);
BOOST_DEFINE_ENUM(test_t, test, test2);

BOOST_TCL_PACKAGE(Enum, "1.0", mod)
{
    auto & cmd_base = boost::tcl::create_command(mod, "enum");

    auto & cmd = cmd_base.add_subcommand("test");
    static int called = 0;
    static bool first = true;
    cmd.add_function(
            +[](foobar fb)
            {
                if (first)
                {
                    called  += 1;
                    assert(fb == foo);
                    first = false;
                }
                else
                {
                    called  += 4;
                    assert(fb == bar);
                }
                return fb;

            });
    cmd.add_function(
            [](test_t d)
            {
                called  += 2;  assert(d == test);
                return d;
            });

    cmd.add_function(
            +[]
            {
                assert(called == 15);
            });
    cmd.add_function(
            +[](boost::string_view sv)
            {
                called  += 8;  assert(sv == "invalid");
                return sv;
            });

    cmd_base.add_subcommand("make").add_enum<foobar>();
    cmd_base.add_subcommand("check")
        .add_function(
                +[](boost::string_view sv)
                {
                    assert(!"must not be called");
                    return sv;
                })
        .add_function(
                +[](foobar fb)
                {
                    assert(foo == fb);
                    return fb;
                });
}