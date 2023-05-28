//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_TCL_EVAL_HPP
#define BOOST_TCL_EVAL_HPP

#include <tcl.h>
#include <boost/tcl/exception.hpp>
#include <boost/tcl/interpreter.hpp>
#include <boost/tcl/object.hpp>

#include <boost/core/detail/string_view.hpp>

namespace boost::tcl
{

template<typename T = object_ptr>
T eval(Tcl_Interp * interp,
       core::string_view script,
       int flags = 0)
{
  auto res = Tcl_EvalEx(interp, script.data(), script.size(), flags);
  if (res != TCL_OK)
    throw_result(interp);
  return cast<T>(interp, Tcl_GetObjResult(interp));
}

template<typename T = object_ptr>
T eval(const interpreter_ptr & interp,
       core::string_view script,
       int flags = 0)
{
  return eval<T>(interp.get(), script, flags);
}

template<typename T = object_ptr>
T eval_file(Tcl_Interp * interp,
            const std::string & file,
            int flags = 0)
{
  auto res = Tcl_EvalFile(interp, file.c_str());
  if (res != TCL_OK)
    throw_result(interp);
  return cast<T>(interp, Tcl_GetObjResult(interp));
}

template<typename T = object_ptr>
T eval_file(const interpreter_ptr & interp,
       const std::string & file)
{
  return eval_file<T>(interp.get(), file);
}


template<typename T = object_ptr>
T eval_file(Tcl_Interp * interp,
            const char * file,
            int flags = 0)
{
  auto res = Tcl_EvalFile(interp, file);
  if (res != TCL_OK)
    throw_result(interp);
  return cast<T>(interp, Tcl_GetObjResult(interp));
}

template<typename T = object_ptr>
T eval_file(const interpreter_ptr & interp,
            const char * file)
{
  return eval_file<T>(interp.get(), file);
}


template<typename T = object_ptr>
T global_eval(Tcl_Interp * interp,
       const char * script)
{
  auto res = Tcl_GlobalEval(interp, script);
  if (res != TCL_OK)
    throw_result(interp);
  return cast<T>(interp, Tcl_GetObjResult(interp));
}

template<typename T = object_ptr>
T global_eval(const interpreter_ptr & interp,
       core::string_view script)
{
  return global_eval<T>(interp.get(), script);
}


template<typename T = object_ptr>
T global_eval(Tcl_Interp * interp,
              const std::string & script)
{
  auto res = Tcl_GlobalEval(interp, script.c_str());
  if (res != TCL_OK)
    throw_result(interp);
  return cast<T>(interp, Tcl_GetObjResult(interp));
}

template<typename T = object_ptr>
T global_eval(const interpreter_ptr & interp,
              const std::string & script)
{
  return global_eval<T>(interp.get(), script);
}



}

#endif //BOOST_TCL_EVAL_HPP
