//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef TCLBIND_EVAL_HPP
#define TCLBIND_EVAL_HPP

#include <tcl.h>
#include <tclbind/exception.hpp>
#include <tclbind/interpreter.hpp>
#include <tclbind/object.hpp>

#include <boost/core/detail/string_view.hpp>
#include <boost/core/span.hpp>

namespace tclbind
{

template<typename T = object_ptr>
result<T> eval(Tcl_Interp * interp,
               boost::core::string_view script,
               int flags = 0)
{
  auto res = Tcl_EvalEx(interp, script.data(), script.size(), flags);
  if (res != TCL_OK)
    return result<T>{boost::system::in_place_error, Tcl_GetObjResult(interp)};;
  return result<T>{boost::system::in_place_value, cast<T>(interp, Tcl_GetObjResult(interp))};
}

template<typename T = object_ptr>
result<T> eval(const interpreter_ptr & interp,
               boost::core::string_view script,
               int flags = 0)
{
  return eval<T>(interp.get(), script, flags);
}


template<typename T = object_ptr>
result<T> eval_tokens(Tcl_Interp * interp,
                      boost::span<Tcl_Token> tokens)
{
  auto res = Tcl_EvalTokensStandard(interp, tokens.data(), tokens.size());
  if (res != TCL_OK)
    return result<T>{boost::system::in_place_error, Tcl_GetObjResult(interp)};;
  return result<T>{boost::system::in_place_value, cast<T>(interp, Tcl_GetObjResult(interp))};
}

template<typename T = object_ptr>
result<T> eval_tokens(const interpreter_ptr & interp,
                      boost::span<Tcl_Token> tokens)
{
  return eval_tokens<T>(interp.get(), tokens);
}



template<typename T = object_ptr>
result<T> eval_file(Tcl_Interp * interp,
                    const std::string & file,
                    int flags = 0)
{
  auto res = Tcl_EvalFile(interp, file.c_str());
  if (res != TCL_OK)
    return result<T>{boost::system::in_place_error, Tcl_GetObjResult(interp)};;
  return result<T>{boost::system::in_place_value, cast<T>(interp, Tcl_GetObjResult(interp))};
}

template<typename T = object_ptr>
result<T> eval_file(const interpreter_ptr & interp,
                    const std::string & file)
{
  return eval_file<T>(interp.get(), file);
}


template<typename T = object_ptr>
result<T> eval_file(Tcl_Interp * interp,
                    const char * file,
                    int flags = 0)
{
  auto res = Tcl_EvalFile(interp, file);
  if (res != TCL_OK)
    return result<T>{boost::system::in_place_error, Tcl_GetObjResult(interp)};;
  return result<T>{boost::system::in_place_value, cast<T>(interp, Tcl_GetObjResult(interp))};
}

template<typename T = object_ptr>
result<T> eval_file(const interpreter_ptr & interp,
                    const char * file)
{
  return eval_file<T>(interp.get(), file);
}


template<typename T = object_ptr>
result<T> global_eval(Tcl_Interp * interp,
                      const char * script)
{
  auto res = Tcl_GlobalEval(interp, script);
  if (res != TCL_OK)
    return result<T>{boost::system::in_place_error, Tcl_GetObjResult(interp)};;
  return result<T>{boost::system::in_place_value, cast<T>(interp, Tcl_GetObjResult(interp))};
}

template<typename T = object_ptr>
result<T> global_eval(const interpreter_ptr & interp,
                      boost::core::string_view script)
{
  return global_eval<T>(interp.get(), script);
}


template<typename T = object_ptr>
result<T> global_eval(Tcl_Interp * interp,
                      const std::string & script)
{
  auto res = Tcl_GlobalEval(interp, script.c_str());
  if (res != TCL_OK)
    return result<T>{boost::system::in_place_error, Tcl_GetObjResult(interp)};;
  return result<T>{boost::system::in_place_value, cast<T>(interp, Tcl_GetObjResult(interp))};
}

template<typename T = object_ptr>
result<T> global_eval(const interpreter_ptr & interp,
                      const std::string & script)
{
  return global_eval<T>(interp.get(), script);
}


template<>
inline
result<void> eval<void>(Tcl_Interp * interp,
                        boost::core::string_view script,
                        int flags)
{
  auto res = Tcl_EvalEx(interp, script.data(), script.size(), flags);
  if (res != TCL_OK)
    return result<void>{boost::system::in_place_error, Tcl_GetObjResult(interp)};;
  return result<void>{boost::system::in_place_value};
}

template<>
inline
result<void> eval_tokens(Tcl_Interp * interp,
                         boost::span<Tcl_Token> tokens)
{
  auto res = Tcl_EvalTokensStandard(interp, tokens.data(), tokens.size());
  if (res != TCL_OK)
    return result<void>{boost::system::in_place_error, Tcl_GetObjResult(interp)};;
  return result<void>{boost::system::in_place_value};
}

template<>
inline
result<void> eval_file(Tcl_Interp * interp,
                       const std::string & file,
                       int flags)
{
  auto res = Tcl_EvalFile(interp, file.c_str());
  if (res != TCL_OK)
    return result<void>{boost::system::in_place_error, Tcl_GetObjResult(interp)};;
  return result<void>{boost::system::in_place_value};
}

template<>
inline
result<void> eval_file(Tcl_Interp * interp,
                       const char * file,
                       int flags)
{
  auto res = Tcl_EvalFile(interp, file);
  if (res != TCL_OK)
    return result<void>{boost::system::in_place_error, Tcl_GetObjResult(interp)};;
  return result<void>{boost::system::in_place_value};
}

template<>
inline
result<void> global_eval(Tcl_Interp * interp,
                         const char * script)
{
  auto res = Tcl_GlobalEval(interp, script);
  if (res != TCL_OK)
    return result<void>{boost::system::in_place_error, Tcl_GetObjResult(interp)};;
  return result<void>{boost::system::in_place_value};
}

template<>
inline
result<void> global_eval(Tcl_Interp * interp,
                         const std::string & script)
{
  auto res = Tcl_GlobalEval(interp, script.c_str());
  if (res != TCL_OK)
    return result<void>{boost::system::in_place_error, Tcl_GetObjResult(interp)};;
  return result<void>{boost::system::in_place_value};
}

}

#endif //TCLBIND_EVAL_HPP
