//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/tcl.hpp>

#include <boost/asio/write.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/local/stream_protocol.hpp>
#include <boost/scope_exit.hpp>
#include <filesystem>

namespace asio = boost::asio;
namespace tcl = boost::tcl;


struct wrapped_stdio
{
  wrapped_stdio() = delete;
  wrapped_stdio(const wrapped_stdio &) = delete;
  wrapped_stdio(wrapped_stdio &&) = delete;

  wrapped_stdio(asio::local::stream_protocol::socket & socket) : socket(socket) {};

  asio::local::stream_protocol::socket & socket;

  int input(char *buf, int to_read, int &error)
  {
    std::string str{buf, static_cast<std::size_t>(to_read)};
    boost::system::error_code ec;
    auto n = socket.read_some(asio::buffer(buf, to_read), ec);
    error = ec.value();
    return n;
  }

  int output(const char *buf, int to_write, int &error)
  {
    std::string str{buf, static_cast<std::size_t>(to_write)};
    boost::system::error_code ec;
    auto n = socket.write_some(asio::buffer(buf, static_cast<std::size_t>(to_write)), ec);
    error = ec.value();
    return n;
  }

  int seek(std::int64_t offset, int mode, int &error)
  {
    return TCL_ERROR;
  }

  int set_option(Tcl_Interp *interp, const char *name, const char *value)
  {
    return TCL_ERROR;
  }

  int get_option(Tcl_Interp *interp, const char *name, Tcl_DString & string)
  {
    return TCL_ERROR;
  }

  int watch(int mask)
  {
    return TCL_ERROR;
  }

  int close(Tcl_Interp * interp, int side)
  {
    // noop, as we still need to use it.
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

tcl::interpreter_ptr make(asio::any_io_executor exec)
{
  auto ip = tcl::make_interpreter();
  tcl::create_command(ip, "status")
      .add_function(
          [exec]() -> boost::core::string_view
          {
            return "Diagnostics: this is still a minimal program that ain't doing much, so all is good";
          });
  return ip;
}


void handle_session(asio::local::stream_protocol::socket sock)
{

  Tcl_Channel stdio[3] = {Tcl_GetStdChannel(TCL_STDIN), Tcl_GetStdChannel(TCL_STDOUT), Tcl_GetStdChannel(TCL_STDERR)};
  BOOST_SCOPE_EXIT_ALL(&)
    {
      Tcl_SetStdChannel(stdio[0], TCL_STDIN);
      Tcl_SetStdChannel(stdio[1], TCL_STDOUT);
      Tcl_SetStdChannel(stdio[2], TCL_STDERR);
    };

  wrapped_stdio ws{sock};

  auto cn = tcl::create_channel(ws, "socket");
  Tcl_SetChannelBufferSize(cn, 0);

  auto p = make(sock.get_executor());
  Tcl_Init(p.get());
  Tcl_SetStdChannel(cn, TCL_STDIN);
  Tcl_SetStdChannel(cn, TCL_STDOUT);
  Tcl_SetStdChannel(cn, TCL_STDERR);

  Tcl_RegisterChannel(p.get(), cn);
  BOOST_SCOPE_EXIT_ALL(&) { Tcl_UnregisterChannel(p.get(), cn);};
  tcl::eval<void>(p.get(), R"(coroutine _eval_line apply {{} { while {true} {eval [ yield ]} }} )").value();

  std::string buf;
  auto dbuf = asio::dynamic_buffer(buf);

  asio::write(sock, asio::buffer("shell> "));

  while (true)
  {
    auto n = sock.read_some(dbuf.prepare(4096));
    dbuf.commit(n);
    tcl::parse_result pr;
    std::string_view cmd{buf.data(), n};
    auto pre = cmd.find_first_not_of(" \n\r\t");
    if (pre == std::string_view::npos)
    {
      dbuf.consume(cmd.size());
      asio::write(sock, asio::buffer("shell> "));
      continue;
    }

    auto pc = tcl::parse_command(p, pr, cmd, true);

    if (!pc && pr.incomplete)
    {
      std::string resp = tcl::cast<std::string>(p, pc.error());
      resp += "\nshell> ";
      std::cout << pr.command() << std::endl;
      asio::write(sock, asio::buffer(resp));
      continue;
    }
    if (pr.incomplete)
      continue ;

    auto res = tcl::eval(p, pr.command());
    dbuf.consume(pr.command().size());
    std::string resp = tcl::cast<std::string>(p, res ? res.value() : res.error());
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

  while (true)
    try {
      handle_session(acc.accept());
    }
    catch ( boost::system::system_error & se)
    {
      if (se.code() != boost::asio::error::eof)
        printf("Session ended with error [%d] %s\n", se.code().value(), se.what());
      else
        printf("Session ended gracefully\n");
    }


  return 0;
}