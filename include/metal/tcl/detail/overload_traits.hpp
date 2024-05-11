//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef METAL_TCL_DETAIL_OVERLOAD_TRAITS_HPP
#define METAL_TCL_DETAIL_OVERLOAD_TRAITS_HPP

#include <tcl.h>

#include <boost/assert.hpp>
#include <boost/callable_traits.hpp>
#include <cstdint>
#include <tuple>
#include <type_traits>

namespace metal::tcl::detail
{

template<typename Func>
struct overload_traits
{
  using args_type = boost::callable_traits::args_t<Func>;
  using return_type = boost::callable_traits::return_type_t<Func>;

  constexpr static std::size_t cnt = std::tuple_size<args_type>::value;
  constexpr static std::make_index_sequence<cnt> seq{};
  using function_type = std::remove_pointer_t<Func>*;

  template<std::size_t Idx>
  using type = std::tuple_element_t<Idx, args_type>;

  template<typename ... Opts>
  static int try_invoke_no_string_impl(function_type func, Tcl_Interp * interp,
                                       std::false_type, Opts && ... opts)
  {
    const bool invocable = (!!opts && ...);
    if (invocable)
    {
      auto res = (*func)(*std::move(opts)...);
      auto obj = make_object(interp, std::move(res));
      Tcl_SetObjResult(interp, obj.get());
      return TCL_OK;
    }
    else
      return TCL_CONTINUE;
  }

  template<typename ... Opts>
  static int try_invoke_no_string_impl(function_type func, Tcl_Interp * interp,
                                       std::true_type,  Opts && ... opts)
  {
    const bool invocable = (!!opts && ...);
    if (invocable)
    {
      (*func)(*std::move(opts)...);
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
        func, interp, std::is_void<return_type>{},
        // if you get an error you're missing some tag_invokes.
        try_cast_without_implicit_string<type<Idx>>(interp, objv[Idx + 1])...);
  }


  template<typename ... Opts>
  static int invoke_impl(function_type func,
                         Tcl_Interp  * interp,
                         std::false_type, /* is void */
                         Opts && ... opts)
  {
    const bool invocable = (!!opts && ...);
    if (invocable)
    {
      auto res = (*func)(*std::move(opts)...);
      auto obj = make_object(interp, std::move(res));
      Tcl_SetObjResult(interp, obj.get());

      return TCL_OK;
    }
    else
      return TCL_CONTINUE;
  }

  template<typename ... Opts>
  static int invoke_impl(function_type func,
                         Tcl_Interp  * interp,
                         std::true_type, /* is void */
                         Opts && ... opts)
  {
    const bool invocable = (!!opts && ...);
    if (invocable)
    {
      (*func)(*std::move(opts)...);
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
        func, interp, std::is_void<return_type>{},
        try_cast<type<Idx>>(interp, objv[Idx + 1])...);
  }

  template<std::size_t ... Idx>
  static int call_equal_impl(function_type func, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[],
                             std::index_sequence<Idx...> seq = {})
  {
    const bool all_equal = (is_equal_type<type<Idx>>(interp, objv[Idx + 1]) && ... );
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

  static int call_equal(function_type func, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
  {
    BOOST_ASSERT(objc == cnt + 1);
    return call_equal_impl(func, interp, objc, objv, seq);
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

  static int call_equivalent(function_type func, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
  {
    BOOST_ASSERT(objc == cnt + 1);
    return call_equivalent_impl(func, interp, objc, objv, seq);
  }

  static int call_castable(void * func, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
  {
    const auto p = reinterpret_cast<function_type>(func);
    BOOST_ASSERT(objc == cnt + 1);
    return try_invoke_no_string(p, interp, objc, objv, seq);
  }

  static int call_castable(function_type func, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
  {
    BOOST_ASSERT(objc == cnt + 1);
    return try_invoke_no_string(func, interp, objc, objv, seq);
  }

  static int call_with_string(void * func, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
  {
    const auto p = reinterpret_cast<function_type>(func);
    BOOST_ASSERT(objc == cnt + 1);
    return invoke(p, interp, objc, objv, seq);
  }

  static int call_with_string(function_type func, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
  {
    BOOST_ASSERT(objc == cnt + 1);
    return invoke(func, interp, objc, objv, seq);
  }
};

// same as above, but with interpreter

template<typename Func>
struct overload_traits_with_interp
{
  using args_type = boost::mp11::mp_drop_c<boost::callable_traits::args_t<Func>, 1u>;
  using return_type = boost::callable_traits::return_type_t<Func>;

  constexpr static std::size_t cnt = std::tuple_size<args_type>::value;
  constexpr static std::make_index_sequence<cnt> seq{};
  using function_type = Func*;

  template<std::size_t Idx>
  using type = std::tuple_element_t<Idx, args_type>;

  template<typename ... Opts>
  static int try_invoke_no_string_impl(function_type func, Tcl_Interp * interp,
                                       std::false_type /* is void */, Opts && ... opts)
  {
    const bool invocable = (!!opts && ...);
    if (invocable)
    {
      auto res = (*func)(interp, *std::move(opts)...);
      auto obj = make_object(interp, std::move(res));
      Tcl_SetObjResult(interp, obj.get());
      return TCL_OK;
    }
    else
      return TCL_CONTINUE;
  }

  template<typename ... Opts>
  static int try_invoke_no_string_impl(function_type func, Tcl_Interp  * interp,
                                       std::true_type /* is void */, Opts && ... opts)
  {
    const bool invocable = (!!opts && ...);
    if (invocable)
    {
      (*func)(interp, *std::move(opts)...);
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
        func, interp, std::is_void<return_type>{},
        // if you get an error you're missing some tag_invokes.
        try_cast_without_implicit_string<type<Idx>>(interp, objv[Idx + 1])...);
  }

  template<typename ... Opts>
  static int invoke_impl(function_type func, Tcl_Interp * interp,
                         std::false_type /* is void */, Opts && ... opts)
  {
    const bool invocable = (!!opts && ...);
    if (invocable)
    {
      auto res = (*func)(interp, *std::move(opts)...);
      auto obj = make_object(interp, std::move(res));
      Tcl_SetObjResult(interp, obj.get());
      return TCL_OK;
    }
    else
      return TCL_CONTINUE;
  }

  template<typename ... Opts>
  static int invoke_impl(function_type func, Tcl_Interp  * interp,
                         std::true_type /* is void */, Opts && ... opts)
  {
    const bool invocable = (!!opts && ...);
    if (invocable)
    {
      (*func)(interp, *std::move(opts)...);
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
        func, interp, std::is_void<return_type>{},
        try_cast<type<Idx>>(interp, objv[Idx + 1])...);
  }

  template<std::size_t ... Idx>
  static int call_equal_impl(function_type func, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[],
                             std::index_sequence<Idx...> seq = {})
  {
    const bool all_equal = (is_equal_type<type<Idx>>(interp, objv[Idx + 1]) && ... );
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


template<typename Func, Func func>
struct member_overload_traits
{
  using args_type = boost::callable_traits::args_t<Func>;
  constexpr static std::size_t cnt = std::tuple_size<args_type>::value;
  using function_type = boost::callable_traits::function_type_t<Func>*;
  using return_type = boost::callable_traits::return_type_t<Func>;
  constexpr static std::make_index_sequence<cnt> seq{};

  template<std::size_t Idx>
  using type = std::tuple_element_t<Idx, args_type>;
  using class_type = type<0u>;

  template<typename Cl, typename ... Opts>
  static int try_invoke_no_string_impl(Tcl_Interp * interp,
                                       std::false_type, Cl * cl, Opts && ... opts)
  {
    const bool invocable = (!!opts && ...);
    if (invocable)
    {
      auto res = (cl->*func)(*std::move(opts)...);
      auto obj = make_object(interp, std::move(res));
      Tcl_SetObjResult(interp, obj.get());
      return TCL_OK;
    }
    else
      return TCL_CONTINUE;
  }

  template<typename Cl, typename ... Opts>
  static int try_invoke_no_string_impl(Tcl_Interp * interp,
                                       std::true_type, Cl * cl, Opts && ... opts)
  {
    const bool invocable = (!!opts && ...);
    if (invocable)
    {
      (cl->*func)(*std::move(opts)...);
      return TCL_OK;
    }
    else
      return TCL_CONTINUE;
  }

  template<std::size_t ... Idx>
  static int try_invoke_no_string(Tcl_Interp * interp, int objc, Tcl_Obj * const objv[],
                                  std::index_sequence<0u, Idx...> seq = {})
  {
    return try_invoke_no_string_impl(
        interp, std::is_void<return_type>{},
        // if you get an error you're missing some tag_invokes.
        static_cast<typename std::remove_reference_t<class_type> *>(objv[0]->internalRep.twoPtrValue.ptr1),
        try_cast_without_implicit_string<type<Idx>>(interp, objv[Idx + 1])...);
  }


  template<typename Cl, typename ... Opts>
  static int invoke_impl(Tcl_Interp  * interp,
                         std::false_type, /* is void */
                         Cl * cl, Opts && ... opts)
  {
    const bool invocable = (!!opts && ...);
    if (invocable)
    {
      auto res = (cl->*func)(*std::move(opts)...);
      auto obj = make_object(interp, std::move(res));
      Tcl_SetObjResult(interp, obj.get());
      return TCL_OK;
    }
    else
      return TCL_CONTINUE;
  }

  template<typename Cl, typename ... Opts>
  static int invoke_impl(Tcl_Interp  * interp,
                         std::true_type, /* is void */
                         Cl * cl, Opts && ... opts)
  {
    const bool invocable = (!!opts && ...);
    if (invocable)
    {
      (cl->*func)(*std::move(opts)...);
      return TCL_OK;
    }
    else
      return TCL_CONTINUE;
  }

  template<std::size_t ... Idx>
  static int invoke(Tcl_Interp * interp, int objc, Tcl_Obj * const objv[],
                    std::index_sequence<0u, Idx...> seq = {})
  {
    return invoke_impl(
        interp, std::is_void<return_type>{},
        static_cast<typename std::remove_reference_t<class_type> *>(objv[0]->internalRep.twoPtrValue.ptr1),
        try_cast<type<Idx>>(interp, objv[Idx + 1])...);
  }

  template<std::size_t ... Idx>
  static int call_equal_impl(Tcl_Interp * interp, int objc, Tcl_Obj * const objv[],
                             std::index_sequence<0u, Idx...> seq = {})
  {
    int idxs[ sizeof ... (Idx)] = {Idx...};
    const bool all_equal = (is_equal_type<type<Idx>>(interp, objv[Idx + 1]) && ... );
    if (all_equal)
      return invoke(interp, objc, objv, seq);
    else
      return TCL_CONTINUE;
  }

  static int call_equal(Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
  {
    auto cc = cnt ;
    if (objc != (cnt + 1))
      return TCL_CONTINUE;
    return call_equal_impl(interp, objc, objv, seq);
  }

  template<std::size_t ... Idx>
  static int call_equivalent_impl(Tcl_Interp * interp, int objc, Tcl_Obj * const objv[],
                                  std::index_sequence<0u, Idx...> seq = {})
  {
    int idxs[ sizeof ... (Idx)] = {Idx...};
    const bool all_equal = (is_equivalent_type<type<Idx>>(objv[Idx + 1]->typePtr) && ... );
    if (all_equal)
      return invoke(interp, objc, objv, seq);
    else
      return TCL_CONTINUE;
  }

  static int call_equivalent(Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
  {
    if (objc != (cnt + 1))
      return TCL_CONTINUE;
    return call_equivalent_impl(interp, objc, objv, seq);
  }

  static int call_castable(Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
  {
    if (objc != (cnt + 1))
      return TCL_CONTINUE;
    return try_invoke_no_string(interp, objc, objv, seq);
  }

  static int call_with_string(Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
  {
    if (objc != (cnt + 1))
      return TCL_CONTINUE;
    return invoke(interp, objc, objv, seq);
  }
};


}

#endif //METAL_TCL_DETAIL_OVERLOAD_TRAITS_HPP
