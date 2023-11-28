//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef TCLBIND_VAR_HPP
#define TCLBIND_VAR_HPP

#include <tcl.h>
#include <tclbind/cast.hpp>
#include <tclbind/exception.hpp>
#include <tclbind/interpreter.hpp>

namespace tclbind
{

inline
const char * set(Tcl_Interp * interp,
                 const char * name,
                 const char * value,
                 bool global_only = false)
{
  return Tcl_SetVar2(interp, name, nullptr, value, global_only ? TCL_GLOBAL_ONLY : 0);
}


template<typename T>
inline
object_ptr set(Tcl_Interp * interp,
                 const char * name,
                 T && value,
                 bool global_only = false)
{
  return Tcl_SetVar2Ex(interp, name, nullptr, make_object(interp, value).detach(), global_only ? TCL_GLOBAL_ONLY : 0);
}

inline
const char * append(Tcl_Interp * interp,
                 const char * name,
                 const char * value,
                 bool global_only = false)
{
  return Tcl_SetVar2(interp, name, nullptr, value, (global_only ? TCL_GLOBAL_ONLY : 0) | TCL_APPEND_VALUE);
}


template<typename T = object_ptr>
inline
T get(Tcl_Interp * interp,
      const char * name,
      bool global_only = false)
{
  auto p = Tcl_GetVar2Ex(interp, name, nullptr, (global_only ? TCL_GLOBAL_ONLY : 0) | TCL_LEAVE_ERR_MSG);
  if (!p)
    throw_result(interp);
  return cast<T>(interp, p);
}


template<typename T = object_ptr>
inline
std::optional<T> try_get(
    Tcl_Interp * interp,
    const char * name,
    bool global_only = false)
{
  auto p = Tcl_GetVar2Ex(interp, name, nullptr,
                         (global_only ? TCL_GLOBAL_ONLY : 0));
  if (!p)
    return std::nullopt;
  return cast<T>(interp, p);
}

inline
bool unset(Tcl_Interp * interp,
           const char * name,
           bool global_only = false)
{
  return Tcl_UnsetVar2(interp, name, nullptr,
                       (global_only ? TCL_GLOBAL_ONLY : 0)) == TCL_OK;
}


inline
const char * set(Tcl_Interp * interp,
                 const char * name,
                 const char * key,
                 const char * value,
                 bool global_only = false)
{
  return Tcl_SetVar2(interp, name, key, value, global_only ? TCL_GLOBAL_ONLY : 0);
}


template<typename T>
inline
object_ptr set(Tcl_Interp * interp,
               const char * name,
               const char * key,
               T && value,
               bool global_only = false)
{
  return Tcl_SetVar2Ex(interp, name, key,
                       make_object(interp, value).detach(),
                       global_only ? TCL_GLOBAL_ONLY : 0);
}

inline
const char * append(Tcl_Interp * interp,
                    const char * name,
                    const char * key,
                    const char * value,
                    bool global_only = false)
{
  return Tcl_SetVar2(interp, name, key, value,
                     (global_only ? TCL_GLOBAL_ONLY : 0) | TCL_APPEND_VALUE);
}


template<typename T = object_ptr>
inline
T get(Tcl_Interp * interp,
      const char * name,
      const char * key,
      bool global_only = false)
{
  auto p = Tcl_GetVar2Ex(interp, name, key, (global_only ? TCL_GLOBAL_ONLY : 0) | TCL_LEAVE_ERR_MSG);
  if (!p)
    throw_result(interp);
  return cast<T>(interp, p);
}


template<typename T = object_ptr>
inline
std::optional<T> try_get(
    Tcl_Interp * interp,
    const char * name,
    const char * key,

    bool global_only = false)
{
  auto p = Tcl_GetVar2Ex(interp, name, key, (global_only ? TCL_GLOBAL_ONLY : 0));
  if (!p)
    return std::nullopt;
  return cast<T>(interp, p);
}

inline
bool unset(Tcl_Interp * interp,
           const char * name,
           const char * key,
           bool global_only = false)
{
  return Tcl_UnsetVar2(interp, name, key, (global_only ? TCL_GLOBAL_ONLY : 0)) == TCL_OK;
}


inline
const char * set(const interpreter_ptr &  interp,
                 const char * name,
                 const char * value,
                 bool global_only = false)
{
  return set(interp.get(), name, value, global_only);
}

template<typename T>
inline
object_ptr set(const interpreter_ptr &  interp,
               const char * name,
               T && value,
               bool global_only = false)
{
  return set(interp, name, std::forward<T>(value), global_only);
}

inline
const char * append(const interpreter_ptr &  interp,
                    const char * name,
                    const char * value,
                    bool global_only = false)
{
  return append(interp.get(), name, value, global_only);
}

template<typename T = object_ptr>
inline
T get(const interpreter_ptr &  interp,
      const char * name,
      bool global_only = false)
{
  return get<T>(interp.get(), name, global_only);
}


template<typename T = object_ptr>
inline
std::optional<T> try_get(
    const interpreter_ptr &  interp,
    const char * name,
    bool global_only = false)
{
  return try_get<T>(interp.get(), name, global_only);
}

inline
bool unset(const interpreter_ptr &  interp,
           const char * name,
           bool global_only = false)
{
  return unset(interp.get(), name, global_only);
}


inline
const char * set(const interpreter_ptr &  interp,
                 const char * name,
                 const char * key,
                 const char * value,
                 bool global_only = false)
{
  return set(interp.get(), name, value, key, global_only);
}

template<typename T>
inline
object_ptr set(const interpreter_ptr &  interp,
               const char * name,
               const char * key,
               T && value,
               bool global_only = false)
{
  return set(interp, name, std::forward<T>(value), key, global_only);
}

inline
const char * append(const interpreter_ptr &  interp,
                    const char * name,
                    const char * key,
                    const char * value,
                    bool global_only = false)
{
  return append(interp.get(), name, value, key, global_only);
}

template<typename T = object_ptr>
inline
T get(const interpreter_ptr &  interp,
      const char * name,
      const char * key,
      bool global_only = false)
{
  return get<T>(interp.get(), name, global_only);
}


template<typename T = object_ptr>
inline
std::optional<T> try_get(
    const interpreter_ptr &  interp,
    const char * name,
    const char * key,
    bool global_only = false)
{
  return try_get<T>(interp.get(), name, key, global_only);
}

inline
bool unset(const interpreter_ptr &  interp,
           const char * name,
           const char * key,
           bool global_only = false)
{
  return unset(interp.get(), name, key, global_only);
}

template<typename Impl>
struct tracer
{
  tracer(const tracer & ) = delete;

  template<typename Impl_>
  tracer(Impl_ && impl, Tcl_Interp * interp, const char * name,
             int flags = TCL_TRACE_READS | TCL_TRACE_WRITES | TCL_TRACE_UNSETS)
             : name(name), flags(flags), interp(interp), impl_(std::forward<Impl_>(impl))
  {
    int res = Tcl_TraceVar2(interp, name, nullptr, flags | TCL_TRACE_RESULT_OBJECT, &proc_impl_, this);
    if (res != TCL_OK)
      throw_result(interp);
  }

  template<typename Impl_>
  tracer(Impl_ && impl, Tcl_Interp * interp, const char * name, const char * key,
         int flags = TCL_TRACE_READS | TCL_TRACE_WRITES | TCL_TRACE_UNSETS )
         : name(name), key(key), flags(flags), interp(interp), impl_(std::forward<Impl_>(impl))
  {
    int res = Tcl_TraceVar2(interp, name, key, flags | TCL_TRACE_RESULT_OBJECT, &proc_impl_, this);
    if (res != TCL_OK)
      throw_result(interp);
  }

  ~tracer()
  {
    Tcl_UntraceVar2(interp, name, key, flags, &proc_impl_, this);
  }

  const char * const name;
  const char * const key{nullptr};

  const int flags;

  Tcl_Interp *interp;

 private:
  Impl impl_;
  static char * proc_impl_(
      ClientData clientData,
      Tcl_Interp *interp,
      const char *name1,
      const char *name2,
      int flags) noexcept
  {
    try
    {
      static_cast<tracer*>(clientData)->impl_(interp, name1, name2, flags);
      return nullptr;
    }
    catch (...)
    {
      return reinterpret_cast<char*>(make_exception_object().detach());
    }
  }
};


template<typename Impl>
tracer(Impl &&, Tcl_Interp *, const char *, int) -> tracer<std::decay_t<Impl>>;

template<typename Impl>
tracer(Impl &&, Tcl_Interp *, const char *, const char *, int) -> tracer<std::decay_t<Impl>>;


}

#endif //TCLBIND_VAR_HPP
