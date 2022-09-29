//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/tcl/builtin.hpp>
#include <boost/tcl/package.hpp>
#include <boost/tcl/command.hpp>
#include <boost/tcl/class.hpp>
#include <boost/describe.hpp>


struct base
{
    int i = 12;
    void test(int i)
    {
        printf("this: %p\n", this);
        i = 42;
    }

    int test()
    {
        printf("this: %p\n", this);
        return i;
    }
};

BOOST_DESCRIBE_STRUCT(base, (), (i, (int()) test, (void (int)) test));

struct test_class : base
{

    int j;
    test_class(int i) : j(i)
    {
        printf("test_class(i=%d, %p)\n", i, this);
    }
    ~test_class()
    {
        printf("~test_class(%p)\n", this);
    }

    test_class(const test_class &tc) : base(tc), j(j)
    {
        printf("test_class(const & test_class = %p) from %p\n", &tc, this);
    }
    static int static_i ;
    static int s_get() { return static_i;}
    static void s_set(int v) { static_i = v;}



};

int test_class::static_i = 100;


BOOST_DESCRIBE_STRUCT(test_class, (base), (j, static_i, s_get, s_set));

BOOST_TCL_PACKAGE(Class, "1.0", mod)
{
    auto & cmd = boost::tcl::create_command(mod, "test-class");
    cmd.add_class<test_class>().add_constructor<test_class(int)>();
}