//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/tcl/channel.hpp>
#include <boost/tcl/eval.hpp>
#include <filesystem>

#include <boost/beast/core/flat_buffer.hpp>

#include "doctest.h"

using namespace boost;

extern Tcl_Interp *interp;

struct test_impl
{
  test_impl() = default;
  test_impl(const test_impl &) = delete;
  test_impl(test_impl &&) = delete;

  beast::flat_buffer buffer;

  int input(char *buf, int to_read, int &error)
  {
    auto sz =  asio::buffer_copy(asio::mutable_buffer(buf, to_read), buffer.data());
    buffer.consume(sz);
    return sz;
  }

  int output(const char *buf, int to_write, int &error)
  {
    auto sz = asio::buffer_copy(buffer.prepare(to_write), asio::const_buffer(buf, to_write));
    buffer.commit(sz);
    return sz;
  }

  int seek(std::int64_t offset, int mode, int &error)
  {
    return TCL_OK;
  }

  int set_option(Tcl_Interp *interp, const char *name, const char *value)
  {
    return TCL_OK;
  }

  int get_option(Tcl_Interp *interp, const char *name, Tcl_DString & string)
  {
    return TCL_OK;

  }

  int watch(int mask)
  {
    return TCL_OK;
  }

  int close(Tcl_Interp * interp, int side)
  {
    return TCL_OK;
  }


  int block_mode(bool blocking)
  {
    return TCL_OK;
  }

  int handler(int interest_mask)
  {
    return TCL_OK;
  }

};


TEST_CASE("channel")
{
  test_impl ti{};

  auto chan = tcl::create_channel(ti, "testchan");
  Tcl_RegisterChannel(interp, chan);

  std::filesystem::path pt{__FILE__};
  auto pp = pt.parent_path() / "channel.tcl";
  CHECK_NOTHROW(boost::tcl::eval_file(interp, pp.string()));

  Tcl_UnregisterChannel(interp, chan);

  CHECK(ti.buffer.size() == 7);
  auto cd = ti.buffer.cdata();
  core::string_view dt{static_cast<const char *>(cd.data()), cd.size() - 1};
  CHECK(dt == "xyz123");
}