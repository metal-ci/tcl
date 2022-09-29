//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "doctest.h"
#include "boost/tcl/class.hpp"

TEST_SUITE_BEGIN("class");

struct copyable {copyable(const copyable &) = default;} ;
struct non_copyable {non_copyable(const non_copyable &) = delete;} ;

static_assert(boost::tcl::detail::duplicate_class<copyable>);
static_assert(!boost::tcl::detail::duplicate_class<non_copyable>);

struct foo {int i;};
struct bar {int j;};

struct foobar : foo, bar
{
    int k;
};

struct foobar_2 : foobar
{
    int l;
};

BOOST_DESCRIBE_STRUCT(foo, (), ());
BOOST_DESCRIBE_STRUCT(bar, (), ());
BOOST_DESCRIBE_STRUCT(foobar, (foo, bar), ());
BOOST_DESCRIBE_STRUCT(foobar_2, (foobar), ());

TEST_CASE("cast")
{
    using boost::tcl::detail::invoke_describe_cast;
    using boost::tcl::detail::invoke_describe_can_cast;
    const auto impl = & boost::tcl::detail::describe_cast<foobar_2>;
    const auto can_ = & boost::tcl::detail::describe_can_cast<foobar_2>;
    foobar_2 fb;

    CHECK(invoke_describe_can_cast<foobar*>(can_));
    CHECK(invoke_describe_can_cast<foo*>(can_));
    CHECK(invoke_describe_can_cast<bar*>(can_));
    CHECK(!invoke_describe_can_cast<int*>(can_));

    CHECK(static_cast<foobar*>(&fb) == invoke_describe_cast<foobar*>(&fb, impl));
    CHECK(static_cast<foo*>(&fb)    == invoke_describe_cast<foo*>(&fb, impl));
    CHECK(static_cast<bar*>(&fb)    == invoke_describe_cast<bar*>(&fb, impl));
    CHECK(nullptr                   == invoke_describe_cast<int*>(&fb, impl));
}

TEST_SUITE_END();