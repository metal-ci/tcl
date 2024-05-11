//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef METAL_TCL_STRING_COMMAND_HPP
#define METAL_TCL_STRING_COMMAND_HPP

#include <metal/tcl/interpreter.hpp>
#include <boost/callable_traits/return_type.hpp>
#include <boost/callable_traits/args.hpp>

namespace metal::tcl
{

namespace detail
{

template<typename Func>
Tcl_CmdProc* raw_string_command_impl(
    void *,
    std::tuple<int, const char**> * )
{
  return
      +[](ClientData clientData, Tcl_Interp *interp, int argc, CONST84 char *argv[])
      {
        try
        {
          (*static_cast<Func*>(clientData))(argc, argv);
          return TCL_OK;
        }
        catch (...) {
          auto obj = ::metal::tcl::make_exception_object();
          Tcl_SetObjResult(interp, obj.get());
          return TCL_ERROR;
        }
      };
}


template<typename Func, typename Return>
Tcl_CmdProc* raw_string_command_impl(
    Return *,
    std::tuple<int, const char**> * )
{
  return
      +[](ClientData clientData, Tcl_Interp *interp, int argc, CONST84 char *argv[])
      {
        try
        {
          auto obj = make_object(interp, (*static_cast<Func*>(clientData))(argc, argv));
          Tcl_SetObjResult(interp, obj.get());
          return TCL_OK;
        }
        catch (...) {
          auto obj = ::metal::tcl::make_exception_object();
          Tcl_SetObjResult(interp, obj.get());
          return TCL_ERROR;
        }
      };
}


template<typename Func>
Tcl_CmdProc* raw_string_command_impl(
    void *,
    std::tuple<Tcl_Interp*, int, const char**> * )
{
  return
      +[](ClientData clientData, Tcl_Interp *interp, int argc, CONST84 char *argv[])
      {
        try
        {
          (*static_cast<Func*>(clientData))(interp, argc, argv);
          return TCL_OK;
        }
        catch (...) {
          auto obj = ::metal::tcl::make_exception_object();
          Tcl_SetObjResult(interp, obj.get());
          return TCL_ERROR;
        }
      };
}


template<typename Func, typename Return>
Tcl_CmdProc* raw_string_command_impl(
    Return *,
    std::tuple<Tcl_Interp*, int, const char**> * )
{
  return
      +[](ClientData clientData, Tcl_Interp *interp, int argc, CONST84 char *argv[])
      {
        try
        {
          auto obj = make_object(interp, (*static_cast<Func*>(clientData))(interp, argc, argv));
          Tcl_SetObjResult(interp, obj.get());
          return TCL_OK;
        }
        catch (...) {
          auto obj = ::metal::tcl::make_exception_object();
          Tcl_SetObjResult(interp, obj.get());
          return TCL_ERROR;
        }
      };
}
}


template<typename Func>
inline void create_string_command(Tcl_Interp  *interp, const char * name, Func && func)
{
  using func_type = typename std::decay<Func>::type;
  Tcl_CreateCommand(
      interp,
      name,
      detail::raw_string_command_impl<func_type>(
          static_cast<boost::callable_traits::return_type_t<func_type>*>(nullptr),
          static_cast<boost::callable_traits::args_t<func_type>*>(nullptr)
      ),
      new func_type(std::forward<Func>(func)),
      +[](ClientData cdata) { delete static_cast<func_type*>(cdata); });
}

template<typename Func>
inline void create_string_command(const interpreter_ptr & interp, const char * name, Func && func)
{
  create_string_command(interp.get(), name, std::forward<Func>(func()));
}

}

#endif //METAL_TCL_STRING_COMMAND_HPP
