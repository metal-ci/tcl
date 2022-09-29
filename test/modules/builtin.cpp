//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/tcl/builtin.hpp>
#include <boost/tcl/package.hpp>
#include <boost/tcl/command.hpp>


BOOST_TCL_PACKAGE(Builtin, "1.0", mod)
{
    auto & cmd = boost::tcl::create_command(mod, "test-num");

    static int called = 0;

    cmd.add_function(+[](int i)                 {called  += 1;  assert(i == 42);      return i;});
    cmd.add_function(+[](double d)              {called  += 2;  assert(d == 3.142);   return d;});
    cmd.add_function(+[](boost::string_view sv) {called  += 4;  assert(sv == "f123"); return sv;});
    cmd.add_function(+[](bool v)                {called  += 8;  assert(v  == true);   return v;});
    cmd.add_function(+[](boost::tcl::bignum v)
            {
                called  += 16;
                assert(v  == boost::tcl::bignum(42) * 1000000 * 1000000 * 1000000);
                return v;
            });
    cmd.add_function(+[](boost::span<unsigned char> b)
            {
                called  += 32;
                assert(b[0] == 0x01);
                assert(b[1] == 0x23);
                assert(b[2] == 0x45);
                assert(b[3] == 0x67);
                assert(b.size() == 4u);
                return b;
            });

    cmd.add_function(
            +[](std::vector<int> vec)
            {
                called += 64;
                assert(vec[0] == 1);
                assert(vec[1] == 2);
                assert(vec[2] == 3);
                assert(vec[3] == 4);
                assert(vec[4] == 5);
                return vec;
            });

    cmd.add_function(
            +[](std::unordered_map<std::string, double> mp)
            {
                called += 128;
                assert(mp["pi"] == 3.142);
                assert(mp["answer"] == 4.2);
                assert(mp.size() == 2u);
                return mp;
            });

    cmd.add_function(+[] { assert(called == 255); });
}