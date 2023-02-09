// Copyright (c) 2022 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#define DOCTEST_CONFIG_IMPLEMENT

#include <tcl.h>
#include "doctest.h"


Tcl_Interp *interp;

int main(int argc, char** argv)
{
  Tcl_FindExecutable(argv[0]);
  interp = Tcl_CreateInterp();
  if (Tcl_Init(interp) != TCL_OK)
  {
    fprintf (stderr ,"Tcl_Init error: %s\n" ,Tcl_GetStringResult (interp));
    exit(EXIT_FAILURE);
  }
  auto res =  doctest::Context(argc, argv).run();

  Tcl_Finalize();
  exit(EXIT_SUCCESS);

  return res;
}
