//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef TCLBIND_INTERPRETER_HPP
#define TCLBIND_INTERPRETER_HPP

#include <tcl.h>
#include <memory>

namespace tclbind
{

struct interpreter_deleter
{
    void operator()(Tcl_Interp * interp)
    {
        Tcl_DeleteInterp(interp);
    }
};

using interpreter_ptr  = std::unique_ptr<Tcl_Interp, interpreter_deleter>;

inline interpreter_ptr make_interpreter()
{
    return std::unique_ptr<Tcl_Interp, interpreter_deleter>(Tcl_CreateInterp());
}

}

#endif //TCLBIND_INTERPRETER_HPP
