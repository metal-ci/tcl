//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <tclbind/package.hpp>
#include <exception>
#include <stdexcept>

TCLBIND_PACKAGE(Load_error, "1.0", mod)
{
    throw std::runtime_error("my-load-error");
}