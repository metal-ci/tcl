//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_TCL_PROC_HPP
#define BOOST_TCL_PROC_HPP

#include "boost/tcl/cast.hpp"
#include "boost/tcl/exception.hpp"
#include "boost/tcl/object.hpp"

namespace boost::tcl
{

namespace detail
{
template<typename ...>
using object_ptr_t = object_ptr;

}

struct proc
{
  explicit proc(Tcl_Interp * interp, boost::tcl::object_ptr obj) : interp_(interp), obj_(std::move(obj))
  {
    BOOST_ASSERT(interp_);
    BOOST_ASSERT(obj_.get());
  }

  template<typename ... Args>
  auto operator()(Args && ... args)
    -> detail::object_ptr_t<decltype(make_object(static_cast<Tcl_Interp*>(nullptr), args))...>
  {
    std::array<object_ptr, sizeof...(args)> arg_ = {make_object(interp_, std::forward<Args>(args))...};
    Tcl_Obj * argv[sizeof...(args) + 1] = {obj_.get()};
    std::transform(arg_.begin(), arg_.end(), &argv[1], [](const object_ptr & o) {return o.get();});
    if (TCL_OK != Tcl_EvalObjv(interp_, sizeof...(args) + 1, argv, 0))
      throw_result(interp_);
    return Tcl_GetObjResult(interp_);
  }

  const boost::tcl::object_ptr & object() const {return obj_;}

 private:
  Tcl_Interp * interp_;
  boost::tcl::object_ptr obj_;
};

inline std::optional<proc> tag_invoke(
    cast_tag<proc>,
    Tcl_Interp * interp,
    boost::intrusive_ptr<Tcl_Obj> val)
{
  BOOST_ASSERT(interp != nullptr);
  if (nullptr != Tcl_FindCommand(interp, val->bytes, nullptr, 0))
    return proc{interp, std::move(val)};
  else
    return std::nullopt;
}

inline object_ptr tag_invoke(const struct convert_tag &, Tcl_Interp*, const proc & p)
{
  return p.object();
}

}

#endif //BOOST_TCL_PROC_HPP
