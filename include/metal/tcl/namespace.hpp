//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef METAL_TCL_NAMESPACE_HPP
#define METAL_TCL_NAMESPACE_HPP

#include <tcl.h>
#include <metal/tcl/exception.hpp>

#include <utility>

namespace metal::tcl
{
namespace detail
{

template<typename T>
void namespace_deleter(ClientData clientData)
{
  delete static_cast<T*>(clientData);
}

}

template<typename T, typename ... Args>
Tcl_Namespace* create_namespace(Tcl_Interp *interp, const char *name, Args && ... args)
{
  return Tcl_CreateNamespace(interp, name, new T(std::forward<Args>(args)...), &detail::namespace_deleter<T>);
}

template<typename T>
T * get_if(Tcl_Namespace * ns)
{
  if (ns->deleteProc == &detail::namespace_deleter<T>)
    return static_cast<T*>(ns->clientData);
  else
    return nullptr;
}


template<typename T>
T & get(Tcl_Namespace & ns)
{
  if (ns.deleteProc != &detail::namespace_deleter<T>)
    boost::throw_exception(tcl_exception(Tcl_NewStringObj("invalid type for namespace requested", -1)));
  return *static_cast<T*>(ns.clientData);
}

}

#endif //METAL_TCL_NAMESPACE_HPP
