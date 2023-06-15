//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_TCL_EXPR_HPP
#define BOOST_TCL_EXPR_HPP

#include <tcl.h>
#include <boost/tcl/cast.hpp>
#include <boost/tcl/interpreter.hpp>
#include <boost/tcl/object.hpp>
#include <boost/tcl/exception.hpp>

#include <boost/system/result.hpp>

namespace boost::tcl
{

template<typename T = object_ptr,
         typename U>
result<T> expr(Tcl_Interp * interp,
       U && value)
{
  Tcl_Obj * objOut = nullptr;
  auto res = Tcl_ExprObj(interp, make_object(interp, std::forward<U>(value)).detach(), &objOut);

  if (res != TCL_OK)
    return result<T>{system::in_place_error, Tcl_GetObjResult(interp)};

  return result<T>{system::in_place_value, cast<T>(interp, boost::intrusive_ptr<Tcl_Obj>{objOut})};
}


template<typename T = object_ptr,
    typename U>
result<T> expr(const interpreter_ptr & interp,
       U && value)
{
  return expr<T>(interp.get(), std::forward<U>(value));
}

}

#endif //BOOST_TCL_EXPR_HPP
