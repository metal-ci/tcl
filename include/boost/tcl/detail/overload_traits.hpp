//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_TCL_DETAIL_OVERLOAD_TRAITS_HPP
#define BOOST_TCL_DETAIL_OVERLOAD_TRAITS_HPP

#include <tcl.h>

#include <boost/assert.hpp>
#include <cstdint>
#include <tuple>
#include <type_traits>

namespace boost::tcl::detail
{


template<typename Func>
struct overload_traits;

template<typename Return, typename ... Args>
struct overload_traits<Return(Args...)>
{
  constexpr static std::size_t cnt = sizeof...(Args);
  constexpr static std::make_index_sequence<cnt> seq{};
  using function_type = Return(*)(Args...);

  template<std::size_t Idx>
  using type = std::tuple_element_t<Idx, std::tuple<Args...>>;

  template<typename ... Opts>
  static int try_invoke_no_string_impl(function_type func,
                                       std::enable_if_t<!std::is_void_v<decltype(func(*std::declval<Opts>()...))>,  Tcl_Interp> * interp,
  Opts && ... opts)
  {
    const bool invocable = (!!opts && ...);
    if (invocable)
    {
      auto res = func(*std::move(opts)...);
      auto obj = make_object(interp, std::move(res));
      Tcl_SetObjResult(interp, obj.get());
      return TCL_OK;
    }
    else
      return TCL_CONTINUE;
  }

  template<typename ... Opts>
  static int try_invoke_no_string_impl(function_type func,
                                       std::enable_if_t<std::is_void_v<decltype(func(*std::declval<Opts>()...))>, Tcl_Interp>  * interp,
                                       Opts && ... opts)
  {
    const bool invocable = (!!opts && ...);
    if (invocable)
    {
      func(*std::move(opts)...);
      return TCL_OK;
    }
    else
      return TCL_CONTINUE;
  }

  template<std::size_t ... Idx>
  static int try_invoke_no_string(function_type func, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[],
                                  std::index_sequence<Idx...> seq = {})
  {
    return try_invoke_no_string_impl(
        func, interp,
        // if you get an error you're missing some tag_invokes.
        try_cast_without_implicit_string<type<Idx>>(interp, objv[Idx + 1])...);
  }


  template<typename ... Opts>
  static int invoke_impl(function_type func,
                         std::enable_if_t<!std::is_void_v<decltype(func(*std::declval<Opts>()...))>, Tcl_Interp>  * interp,
  Opts && ... opts)
  {
    const bool invocable = (!!opts && ...);
    if (invocable)
    {
      auto res = func(*std::move(opts)...);
      auto obj = make_object(interp, std::move(res));
      Tcl_SetObjResult(interp, obj.get());
      return TCL_OK;
    }
    else
      return TCL_CONTINUE;
  }

  template<typename ... Opts>
  static int invoke_impl(function_type func,
                         std::enable_if_t<std::is_void_v<decltype(func(*std::declval<Opts>()...))>, Tcl_Interp>  * interp,
                         Opts && ... opts)
  {
    const bool invocable = (!!opts && ...);
    if (invocable)
    {
      func(*std::move(opts)...);
      return TCL_OK;
    }
    else
      return TCL_CONTINUE;
  }


  template<std::size_t ... Idx>
  static int invoke(function_type func, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[],
                    std::index_sequence<Idx...> seq = {})
  {
    return invoke_impl(
        func, interp,
        try_cast<type<Idx>>(interp, objv[Idx + 1])...);
  }

  template<std::size_t ... Idx>
  static int call_equal_impl(function_type func, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[],
                             std::index_sequence<Idx...> seq = {})
  {
    const bool all_equal = (is_equal_type<type<Idx>>(objv[Idx + 1]->typePtr) && ... );
    if (all_equal)
      return invoke(func, interp, objc, objv, seq);
    else
      return TCL_CONTINUE;
  }

  template<typename Func>
  static int call_equal(Func * func, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
  {
    const auto p = reinterpret_cast<function_type>(func);
    BOOST_ASSERT(objc == cnt + 1);
    return call_equal_impl(p, interp, objc, objv, seq);
  }

  template<std::size_t ... Idx>
  static int call_equivalent_impl(function_type func, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[],
                                  std::index_sequence<Idx...> seq = {})
  {
    const bool all_equal = (is_equivalent_type<type<Idx>>(objv[Idx + 1]->typePtr) && ... );
    if (all_equal)
      return invoke(func, interp, objc, objv, seq);
    else
      return TCL_CONTINUE;
  }

  template<typename Func>
  static int call_equivalent(Func * func, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
  {
    const auto p = reinterpret_cast<function_type>(func);
    BOOST_ASSERT(objc == cnt + 1);
    return call_equivalent_impl(p, interp, objc, objv, seq);
  }

  template<typename Func>
  static int call_castable(Func * func, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
  {
    const auto p = reinterpret_cast<function_type>(func);
    BOOST_ASSERT(objc == cnt + 1);
    return try_invoke_no_string(p, interp, objc, objv, seq);
  }

  template<typename Func>
  static int call_with_string(Func * func, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
  {
    const auto p = reinterpret_cast<function_type>(func);
    BOOST_ASSERT(objc == cnt + 1);
    return invoke(p, interp, objc, objv, seq);
  }
};

// same as above, but with interpreter

template<typename Func>
struct overload_traits_with_interp;

template<typename Return, typename Interpreter, typename ... Args>
struct overload_traits_with_interp<Return(Interpreter, Args...)>
{
  constexpr static std::size_t cnt = sizeof...(Args);
  constexpr static std::make_index_sequence<cnt> seq{};
  using function_type = Return(*)(Interpreter, Args...);

  template<std::size_t Idx>
  using type = std::tuple_element_t<Idx, std::tuple<Args...>>;

  template<typename ... Opts>
  static int try_invoke_no_string_impl(function_type func,
                                       std::enable_if_t<!std::is_void_v<decltype(func(std::declval<Tcl_Interp *>(), *std::declval<Opts>()...))>,  Tcl_Interp> * interp,
  Opts && ... opts)
  {
    const bool invocable = (!!opts && ...);
    if (invocable)
    {
      auto res = func(interp, *std::move(opts)...);
      auto obj = make_object(interp, std::move(res));
      Tcl_SetObjResult(interp, obj.get());
      return TCL_OK;
    }
    else
      return TCL_CONTINUE;
  }

  template<typename ... Opts>
  static int try_invoke_no_string_impl(function_type func,
                                       std::enable_if_t<std::is_void_v<decltype(func(std::declval<Tcl_Interp *>(), *std::declval<Opts>()...))>, Tcl_Interp>  * interp,
                                       Opts && ... opts)
  {
    const bool invocable = (!!opts && ...);
    if (invocable)
    {
      func(interp, *std::move(opts)...);
      return TCL_OK;
    }
    else
      return TCL_CONTINUE;
  }

  template<std::size_t ... Idx>
  static int try_invoke_no_string(function_type func, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[],
                                  std::index_sequence<Idx...> seq = {})
  {
    return try_invoke_no_string_impl(
        func, interp,
        // if you get an error you're missing some tag_invokes.
        try_cast_without_implicit_string<type<Idx>>(interp, objv[Idx + 1])...);
  }

  template<typename ... Opts>
  static int invoke_impl(function_type func,
                         std::enable_if_t<!std::is_void_v<decltype(func(std::declval<Tcl_Interp *>(), *std::declval<Opts>()...))>, Tcl_Interp>  * interp,
  Opts && ... opts)
  {
    const bool invocable = (!!opts && ...);
    if (invocable)
    {
      auto res = func(interp, *std::move(opts)...);
      auto obj = make_object(interp, std::move(res));
      Tcl_SetObjResult(interp, obj.get());
      return TCL_OK;
    }
    else
      return TCL_CONTINUE;
  }

  template<typename ... Opts>
  static int invoke_impl(function_type func,
                         std::enable_if_t<std::is_void_v<decltype(func(std::declval<Tcl_Interp *>(), *std::declval<Opts>()...))>, Tcl_Interp>  * interp,
                         Opts && ... opts)
  {
    const bool invocable = (!!opts && ...);
    if (invocable)
    {
      func(interp, *std::move(opts)...);
      return TCL_OK;
    }
    else
      return TCL_CONTINUE;
  }


  template<std::size_t ... Idx>
  static int invoke(function_type func, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[],
                    std::index_sequence<Idx...> seq = {})
  {
    return invoke_impl(
        func, interp,
        try_cast<type<Idx>>(interp, objv[Idx + 1])...);
  }

  template<std::size_t ... Idx>
  static int call_equal_impl(function_type func, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[],
                             std::index_sequence<Idx...> seq = {})
  {
    const bool all_equal = (is_equal_type<type<Idx>>(objv[Idx + 1]->typePtr) && ... );
    if (all_equal)
      return invoke(func, interp, objc, objv, seq);
    else
      return TCL_CONTINUE;
  }

  static int call_equal(void * func, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
  {
    const auto p = reinterpret_cast<function_type>(func);
    BOOST_ASSERT(objc == cnt + 1);
    return call_equal_impl(p, interp, objc, objv, seq);
  }

  template<std::size_t ... Idx>
  static int call_equivalent_impl(function_type func, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[],
                                  std::index_sequence<Idx...> seq = {})
  {
    const bool all_equal = (is_equivalent_type<type<Idx>>(objv[Idx + 1]->typePtr) && ... );
    if (all_equal)
      return invoke(func, interp, objc, objv, seq);
    else
      return TCL_CONTINUE;
  }

  static int call_equivalent(void * func, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
  {
    const auto p = reinterpret_cast<function_type>(func);
    BOOST_ASSERT(objc == cnt + 1);
    return call_equivalent_impl(p, interp, objc, objv, seq);
  }
  static int call_castable(void * func, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
  {
    const auto p = reinterpret_cast<function_type>(func);
    BOOST_ASSERT(objc == cnt + 1);
    return try_invoke_no_string(p, interp, objc, objv, seq);
  }

  static int call_with_string(void * func, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
  {
    const auto p = reinterpret_cast<function_type>(func);
    BOOST_ASSERT(objc == cnt + 1);
    return invoke(p, interp, objc, objv, seq);
  }
};

template<typename Pointer, Pointer p>
struct member_overload_traits;

template<typename Return, typename Class,  typename ... Args,
    Return(Class::* Pointer)(Args...)>
struct member_overload_traits<Return(Class::* const)(Args...), Pointer>
    : overload_traits<Return(Class&, Args...)>
{
  typedef Return(Class::*function_type)(Args...);
  constexpr static std::size_t cnt = sizeof...(Args);
  constexpr static std::make_index_sequence<cnt> seq{};

  template<std::size_t Idx>
  using type = std::tuple_element_t<Idx, std::tuple<Args...>>;

  template<typename ... Opts>
  static int try_invoke_no_string_impl(Class *cl,
                                       std::enable_if_t<!std::is_void_v<decltype((cl->*Pointer)(*std::declval<Opts>()...))>,  Tcl_Interp> * interp,
  Opts && ... opts)
  {
    const bool invocable = (!!opts && ...);
    if (invocable)
    {
      auto res = (cl->*Pointer)(*std::move(opts)...);
      auto obj = make_object(interp, std::move(res));
      Tcl_SetObjResult(interp, obj.get());
      return TCL_OK;
    }
    else
      return TCL_CONTINUE;
  }

  template<typename ... Opts>
  static int try_invoke_no_string_impl(Class *cl,
                                       std::enable_if_t<std::is_void_v<decltype((cl->*Pointer)(*std::declval<Opts>()...))>, Tcl_Interp>  * interp,
                                       Opts && ... opts)
  {
    const bool invocable = (!!opts && ...);
    if (invocable)
    {
      (cl->*Pointer)(*std::move(opts)...);
      return TCL_OK;
    }
    else
      return TCL_CONTINUE;
  }

  template<std::size_t ... Idx>
  static int try_invoke_no_string(Class *cl,
                                  Tcl_Interp * interp, int objc, Tcl_Obj * const objv[],
                                  std::index_sequence<Idx...> seq = {})
  {
    return try_invoke_no_string_impl(
        cl, interp,
        // if you get an error you're missing some tag_invokes.
        try_cast_without_implicit_string<type<Idx>>(interp, objv[Idx + 1])...);
  }


  template<typename ... Opts>
  static int invoke_impl(Class *cl,
                         std::enable_if_t<!std::is_void_v<decltype((cl->*Pointer)(*std::declval<Opts>()...))>, Tcl_Interp>  * interp,
  Opts && ... opts)
  {
    const bool invocable = (!!opts && ...);
    if (invocable)
    {
      auto res = (cl->*Pointer)(*std::move(opts)...);
      auto obj = make_object(interp, std::move(res));
      Tcl_SetObjResult(interp, obj.get());
      return TCL_OK;
    }
    else
      return TCL_CONTINUE;
  }

  template<typename ... Opts>
  static int invoke_impl(Class *cl,
                         std::enable_if_t<std::is_void_v<decltype((cl->*Pointer)(*std::declval<Opts>()...))>, Tcl_Interp>  * interp,
                         Opts && ... opts)
  {
    const bool invocable = (!!opts && ...);
    if (invocable)
    {
      (cl->*Pointer)(*std::move(opts)...);
      return TCL_OK;
    }
    else
      return TCL_CONTINUE;
  }


  template<std::size_t ... Idx>
  static int invoke(Class *cl,
                    Tcl_Interp * interp, int objc, Tcl_Obj * const objv[],
                    std::index_sequence<Idx...> seq = {})
  {
    return invoke_impl(cl, interp,
                       try_cast<type<Idx>>(interp, objv[Idx + 1])...);
  }


  template<std::size_t ... Idx>
  static int call_equal_impl(Class *cl,
                             Tcl_Interp * interp, int objc, Tcl_Obj * const objv[],
                             std::index_sequence<Idx...> seq = {})
  {
    const bool all_equal = (is_equal_type<type<Idx>>(objv[Idx + 1]->typePtr) && ... );
    if (all_equal)
      return invoke(cl, interp, objc, objv, seq);
    else
      return TCL_CONTINUE;
  }

  static int call_equal(Class *cl,Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
  {
    return call_equal_impl(cl, interp, objc-1, objv+1, seq);
  }

  static int call_equivalent(Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
  {
    return overload_traits<Return(Class&, Args...)>::call_equivalent(
        +[](Class & cl, Args ... args)
        {
          return (cl.*Pointer)(static_cast<Args>(args)...);
        }, interp, objc-1, objv+1);
  }

  static int call_castable(Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
  {
    return overload_traits<Return(Class&, Args...)>::call_castable(
        +[](Class & cl, Args ... args)
        {
          return (cl.*Pointer)(static_cast<Args>(args)...);
        }, interp, objc-1, objv+1);
  }

  static int call_with_string(Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
  {
    return overload_traits<Return(Class&, Args...)>::call_with_string(
        +[](Class & cl, Args ... args)
        {
          return (cl.*Pointer)(static_cast<Args>(args)...);
        }, interp, objc-1, objv+1);
  }
};


template<typename Return, typename Class,  typename ... Args,
    Return(Class::* Pointer)(Args...) const>
struct member_overload_traits<Return(Class::* const)(Args...) const, Pointer>
    : overload_traits<Return(const Class&, Args...)>
{
  typedef Return(Class::*function_type)(Args...);
  constexpr static std::size_t cnt = sizeof...(Args);
  constexpr static std::make_index_sequence<cnt> seq{};

  template<std::size_t Idx>
  using type = std::tuple_element_t<Idx, std::tuple<Args...>>;

  template<typename ... Opts>
  static int try_invoke_no_string_impl(const Class *cl,
                                       std::enable_if_t<!std::is_void_v<decltype((cl->*Pointer)(*std::declval<Opts>()...))>,  Tcl_Interp> * interp,
  Opts && ... opts)
  {
    const bool invocable = (!!opts && ...);
    if (invocable)
    {
      auto res = (cl->*Pointer)(*std::move(opts)...);
      auto obj = make_object(interp, std::move(res));
      Tcl_SetObjResult(interp, obj.get());
      return TCL_OK;
    }
    else
      return TCL_CONTINUE;
  }

  template<typename ... Opts>
  static int try_invoke_no_string_impl(const Class *cl,
                                       std::enable_if_t<std::is_void_v<decltype((cl->*Pointer)(*std::declval<Opts>()...))>, Tcl_Interp>  * interp,
                                       Opts && ... opts)
  {
    const bool invocable = (!!opts && ...);
    if (invocable)
    {
      (cl->*Pointer)(*std::move(opts)...);
      return TCL_OK;
    }
    else
      return TCL_CONTINUE;
  }

  template<std::size_t ... Idx>
  static int try_invoke_no_string(const Class *cl,
                                  Tcl_Interp * interp, int objc, Tcl_Obj * const objv[],
                                  std::index_sequence<Idx...> seq = {})
  {
    return try_invoke_no_string_impl(
        cl, interp,
        // if you get an error you're missing some tag_invokes.
        try_cast_without_implicit_string<type<Idx>>(interp, objv[Idx + 1])...);
  }


  template<typename ... Opts>
  static int invoke_impl(const Class *cl,
                         std::enable_if_t<!std::is_void_v<decltype((cl->*Pointer)(*std::declval<Opts>()...))>, Tcl_Interp>  * interp,
  Opts && ... opts)
  {
    const bool invocable = (!!opts && ...);
    if (invocable)
    {
      auto res = (cl->*Pointer)(*std::move(opts)...);
      auto obj = make_object(interp, std::move(res));
      Tcl_SetObjResult(interp, obj.get());
      return TCL_OK;
    }
    else
      return TCL_CONTINUE;
  }

  template<typename ... Opts>
  static int invoke_impl(const Class *cl,
                         std::enable_if_t<std::is_void_v<decltype((cl->*Pointer)(*std::declval<Opts>()...))>, Tcl_Interp>  * interp,
                         Opts && ... opts)
  {
    const bool invocable = (!!opts && ...);
    if (invocable)
    {
      (cl->*Pointer)(*std::move(opts)...);
      return TCL_OK;
    }
    else
      return TCL_CONTINUE;
  }


  template<std::size_t ... Idx>
  static int invoke(const Class *cl,
                    Tcl_Interp * interp, int objc, Tcl_Obj * const objv[],
                    std::index_sequence<Idx...> seq = {})
  {
    return invoke_impl(cl, interp,
                       try_cast<type<Idx>>(interp, objv[Idx + 1])...);
  }


  template<std::size_t ... Idx>
  static int call_equal_impl(const Class *cl,
                             Tcl_Interp * interp, int objc, Tcl_Obj * const objv[],
                             std::index_sequence<Idx...> seq = {})
  {
    const bool all_equal = (is_equal_type<type<Idx>>(objv[Idx + 1]->typePtr) && ... );
    if (all_equal)
      return invoke(cl, interp, objc, objv, seq);
    else
      return TCL_CONTINUE;
  }

  static int call_equal(const Class *cl,Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
  {
    return call_equal_impl(cl, interp, objc-1, objv+1, seq);
  }

  static int call_equivalent(Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
  {
    return overload_traits<Return(const Class&, Args...)>::call_equivalent(
        +[](const Class & cl, Args ... args)
        {
          return (cl.*Pointer)(static_cast<Args>(args)...);
        }, interp, objc-1, objv+1);
  }

  static int call_castable(Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
  {
    return overload_traits<Return(const Class&, Args...)>::call_castable(
        +[](const Class & cl, Args ... args)
        {
          return (cl.*Pointer)(static_cast<Args>(args)...);
        }, interp, objc-1, objv+1);
  }

  static int call_with_string(Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
  {
    return overload_traits<Return(const Class&, Args...)>::call_with_string(
        +[](const Class & cl, Args ... args)
        {
          return (cl.*Pointer)(static_cast<Args>(args)...);
        }, interp, objc-1, objv+1);
  }
};

}

#endif //BOOST_TCL_DETAIL_OVERLOAD_TRAITS_HPP
