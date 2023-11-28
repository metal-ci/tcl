//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef TCLBIND_EXPR_HPP
#define TCLBIND_EXPR_HPP

#include <tcl.h>
#include <tclbind/cast.hpp>
#include <tclbind/interpreter.hpp>
#include <tclbind/object.hpp>
#include <tclbind/exception.hpp>

#include <boost/system/result.hpp>

namespace tclbind
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

#endif //TCLBIND_EXPR_HPP
