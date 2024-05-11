//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//


// simple CLI that's just a calculator

#include <metal/tcl.hpp>
#include <metal/tcl/builtin/bignum.hpp>

template<typename T> T add(T x, T y) {return x + y;}
template<typename T> T sub(T x, T y) {return x - y;}
template<typename T> T mul(T x, T y) {return x * y;}
template<typename T> T div(T x, T y)
{
  if (y == 0)
    throw std::invalid_argument("division by zero");
  return x / y;
}

int main(int argc, char *argv[])
{
  namespace tcl = metal::tcl;
  auto ip = tcl::make_interpreter();

  tcl::create_command(ip, "add")
            .add_function(&add<int>)
            .add_function(&add<double>)
            .add_function(&add<tcl::bignum>)
            .add_function(&add<Tcl_WideInt>)
            .add_function(&add<Tcl_WideUInt>);

  tcl::create_command(ip, "sub")
            .add_function(&sub<int>)
            .add_function(&sub<double>)
            .add_function(&sub<tcl::bignum>)
            .add_function(&sub<Tcl_WideInt>)
            .add_function(&sub<Tcl_WideUInt>);


  tcl::create_command(ip, "mul")
            .add_function(&mul<int>)
            .add_function(&mul<double>)
            .add_function(&mul<tcl::bignum>)
            .add_function(&mul<Tcl_WideInt>)
            .add_function(&mul<Tcl_WideUInt>);

  tcl::create_command(ip, "div")
            .add_function(&div<int>)
            .add_function(&div<double>)
            .add_function(&div<tcl::bignum>)
            .add_function(&div<Tcl_WideInt>)
            .add_function(&div<Tcl_WideUInt>);

  if (argc > 1)
  {
    std::vector<tcl::object_ptr> vec;

    std::vector<Tcl_Obj *> vobjs;

    vec.reserve(argc-1);
    vobjs.reserve(argc-1);
    for (int i = 1; i < argc; i++)
      vobjs.push_back(
          vec.emplace_back(
              Tcl_NewStringObj(argv[i], std::strlen(argv[i]))
              ).get());

    try
    {
      if (Tcl_EvalObjv(ip.get(), vobjs.size(), vobjs.data(), 0) != TCL_OK)
        tcl::throw_result(ip.get());
    }
    catch (std::exception & e)
    {
        std::cerr << "exception: " << e.what() << std::endl;
        return 1;
    }
    std::cout << Tcl_GetStringResult(ip.get()) << std::endl;
  }
  else // interactive mode -  we force the user to wire single line commands!
  {
    static bool exit = false;
    tcl::create_command(ip, "exit").add_function(+[]{exit = true;});

    std::string line;
    std::cout << "calc> " << std::flush;
    while (!exit && std::getline(std::cin, line))
    {
      try
      {
        if (Tcl_Eval(ip.get(), line.c_str()) != TCL_OK)
          tcl::throw_result(ip.get());
        std::cout << Tcl_GetStringResult(ip.get());
        if (!exit)
          std::cout << "\ncalc> " << std::flush;

      }
      catch (std::exception & e)
      {
        std::cerr << "exception: " << e.what() << std::endl;
        std::cout << "\ncalc> " << std::flush;
      }
    }
  }

  return 0;
}
