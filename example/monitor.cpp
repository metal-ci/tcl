//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/asio/write.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/local/stream_protocol.hpp>
#include <boost/tcl.hpp>

#include <filesystem>

namespace asio = boost::asio;
namespace tcl = boost::tcl;

tcl::interpreter_ptr make()
{
  auto ip = tcl::make_interpreter();
  tcl::create_command(ip, "status")
      .add_function_with_interp(
          [](Tcl_Interp * interp) -> boost::core::string_view
          {
            return "Diagnostics: this is still a minimal program that ain't doing much, so all is good";
          });
  return ip;
}


void handle_session(asio::local::stream_protocol::socket sock)
{
  auto p = make();

  Tcl_Init(p.get());

  std::string buf;
  auto dbuf = asio::dynamic_buffer(buf);

  Tcl_Parse parse;
  asio::write(sock, asio::buffer("shell> "));

  while (true)
  {

    auto n = sock.read_some(dbuf.prepare(4096));
    dbuf.commit(n);

    if (Tcl_ParseCommand(p.get(), buf.data(), n, 1, &parse) != TCL_OK)
    {
      if (!parse.incomplete)
        tcl::throw_result(p.get());
    }

    if (parse.incomplete)
      continue ;
    Tcl_EvalEx(p.get(), parse.commandStart, parse.commandSize, 0);
    dbuf.consume(parse.commandSize);
    std::string resp = Tcl_GetStringResult(p.get());
    resp += "\nshell> ";
    asio::write(sock, asio::buffer(resp));
  }



}

int main(int argc, char * argv[])
{
  Tcl_FindExecutable(argv[0]);

  asio::io_context ctx;

  if (std::filesystem::exists(argv[1]))
    std::filesystem::remove(argv[1]);

  asio::local::stream_protocol::acceptor acc{ctx, argv[1]};

  handle_session(acc.accept());


  return 0;
}