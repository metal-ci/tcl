//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_TCL_ASYNC_HPP
#define BOOST_TCL_ASYNC_HPP

#include <tcl.h>

namespace boost::tcl
{

struct async
{
  async() = default;
  async(const async & ) = delete;

  ~async()
  {
    Tcl_AsyncDelete(handler_);
  }

  virtual int invoke(Tcl_Interp * interp, int code) = 0;
  void mark() { Tcl_AsyncMark(handler_); }

 private:
  Tcl_AsyncHandler handler_{
    Tcl_AsyncCreate(
        +[](ClientData clientData, Tcl_Interp *interp, int code)
        {
          return static_cast<async*>(clientData)->invoke(interp, code);
        }, this)};
};

}

#endif //BOOST_TCL_ASYNC_HPP
