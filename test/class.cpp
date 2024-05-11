//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#define USE_TCL_STUBS
#include <tcl.h>
#define USE_TCLOO_STUBS
#include <tclOO.h>

#include <dlfcn.h>
#include "doctest.h"
#include <metal/tcl/interpreter.hpp>
#include <metal/tcl/builtin.hpp>
#include <metal/tcl/eval.hpp>
#include <metal/tcl/object.hpp>


#include <metal/tcl/class.hpp>
#include <filesystem>

extern Tcl_Interp *interp;

using namespace boost;

struct base
{
  int i = 12;
  void test(int i)
  {
    i = 42;
  }

  int test()
  {
    return i;
  }
  void do_the_thing()
  {

  }
};

BOOST_DESCRIBE_STRUCT(base, (), (i, (int()) test, (void (int)) test, do_the_thing));

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

  test_class(const test_class &tc) : base(tc), j(tc.j)
  {
    printf("test_class(const & test_class = %p) from %p\n", &tc, this);
  }
  static int static_i ;
  static int s_get() { return static_i;}
  static void s_set(int v) { static_i = v;}
};

int test_class::static_i = 100;


BOOST_DESCRIBE_STRUCT(test_class, (base), (j, static_i, s_get, s_set));

METAL_TCL_DESCRIBE_CONSTRUCTORS(test_class, (int));

extern Tcl_Interp *interp;

TEST_SUITE_BEGIN("class");

METAL_TCL_SET_CLASS_NAME(test_class, test-class);

template< const char * const &Name> struct foobar {};

TEST_CASE("cast")
{
  REQUIRE(Tcl_InitStubs(interp , TCL_VERSION ,0) != nullptr);
  REQUIRE(Tcl_OOInitStubs(interp) != nullptr);
  metal::tcl::register_class<test_class>(interp);

  auto p = metal::tcl::make_object(interp, test_class{2});

  std::filesystem::path pt{__FILE__};
  auto pp = pt.parent_path() / "class.tcl";
  metal::tcl::eval_file(interp, pp.string().c_str());
}
TEST_SUITE_END();