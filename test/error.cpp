//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <metal/tcl/exception.hpp>
#include <metal/tcl/interpreter.hpp>
#include <metal/tcl/object.hpp>
#include <metal/tcl/eval.hpp>
#include <string>
#include <stdexcept>

#include "doctest.h"

TEST_SUITE_BEGIN("error");

static int
throw_cmd(ClientData cdata, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
try {
    throw std::runtime_error("test-error");
}
catch (...)
{
    auto obj = metal::tcl::make_exception_object();
    Tcl_SetObjResult(interp, obj.get());
    return TCL_ERROR;
}

extern Tcl_Interp *interp;

TEST_CASE("exception")
{
    {
        Tcl_CreateObjCommand(interp, "throw_test", throw_cmd, nullptr, nullptr);
        CHECK_THROWS_WITH(metal::tcl::eval(interp, R"(throw_test)"), "test-error");
        Tcl_DeleteCommand(interp, "throw_test");

    }
    Tcl_Finalize();
}

TEST_SUITE_END();