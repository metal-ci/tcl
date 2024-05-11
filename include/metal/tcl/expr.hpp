//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef METAL_TCL_EXPR_HPP
#define METAL_TCL_EXPR_HPP

#include <tcl.h>
#include <metal/tcl/cast.hpp>
#include <metal/tcl/interpreter.hpp>
#include <metal/tcl/object.hpp>
#include <metal/tcl/exception.hpp>

#include <boost/system/result.hpp>

namespace metal::tcl
{

template<typename T = object_ptr,
         typename U>
result<T> expr(Tcl_Interp * interp,
       U && value)
{
  Tcl_Obj * objOut = nullptr;
  auto res = Tcl_ExprObj(interp, make_object(interp, std::forward<U>(value)).detach(), &objOut);

  if (res != TCL_OK)
    return result<T>{boost::system::in_place_error, Tcl_GetObjResult(interp)};

  return result<T>{boost::system::in_place_value, cast<T>(interp, boost::intrusive_ptr<Tcl_Obj>{objOut})};
}


template<typename T = object_ptr,
    typename U>
result<T> expr(const interpreter_ptr & interp,
       U && value)
{
  return expr<T>(interp.get(), std::forward<U>(value));
}

}

#endif //METAL_TCL_EXPR_HPP
