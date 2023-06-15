//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/tcl/parse.hpp>

#include "doctest.h"

extern Tcl_Interp *interp;

using namespace boost;

TEST_CASE("parse-command")
{
  tcl::parse_result p{};
  CHECK(!p.incomplete);

  CHECK_THROWS(parse_command(interp, p, "set foo [").value());
  CHECK(p.incomplete);

  CHECK(parse_command(interp, p, "set foo [expr 3 + 1]"));
  CHECK(!p.incomplete);
  CHECK(std::string(p.command()) == "set foo [expr 3 + 1]");

  CHECK(p.tokens().size() == 6);

  CHECK(parse_command(interp, p, "# dummy"));
  CHECK(std::string(p.comment()) == "# dummy");
  CHECK(p.tokens().size() == 0);
}

TEST_CASE("parse-expr")
{
  tcl::parse_result p{};
  CHECK(!p.incomplete);

  CHECK(!parse_expr(interp, p, "3 + 1]"));
  CHECK(parse_expr(interp, p, "3 + 1"));
  CHECK(!p.incomplete);
  CHECK(p.command() == "");
  CHECK(p.comment() == "");

  CHECK(p.command().empty());

  CHECK(p.tokens().size() == 6);

  CHECK(!parse_expr(interp, p, "# dummy"));
  CHECK(p.tokens().size() == 0);
}

TEST_CASE("parse-braces")
{
  tcl::parse_result p{};

  CHECK(!parse_braces(interp, p, "{3 + [string length \"abc"));
  CHECK(p.error_type() != 0);
  CHECK(parse_braces(interp, p, "{3 + 1} asd}").value() == "{3 + 1}");

  CHECK(p.command().empty());

  CHECK(p.tokens().size() == 1);

  CHECK(!parse_braces(interp, p, "# dummy"));
  CHECK(p.tokens().size() == 0);
}


TEST_CASE("parse-quoted-string")
{
  tcl::parse_result p{};
  CHECK(!p.incomplete);

  CHECK(!parse_quoted_string(interp, p, R"(" ab \")"));
  CHECK(p.error_type() == TCL_PARSE_MISSING_QUOTE);
  CHECK(parse_quoted_string(interp, p, R"(" ab \""")") == "\" ab \\\"\"");
  CHECK(!parse_quoted_string(interp, p, "# dummy"));
  CHECK(p.tokens().size() == 1);
}

TEST_CASE("parse-var-name")
{
  tcl::parse_result p{};

  Tcl_SetVar(interp, "abcd", "42", 0);

  CHECK(parse_var_name(interp, p, "ab \"").has_value());
  CHECK(parse_var_name(interp, p, "$ab \"").has_value());
  REQUIRE(p.tokens().size() > 0u);
  auto tk = p.tokens()[0];
  CHECK(p.tokens()[0].type == TCL_TOKEN_VARIABLE);
  CHECK(std::string(tk.start, tk.size) == "$ab");
  CHECK(parse_var_name(interp, p, "$abcd(xyz)"));
  CHECK(std::string(p.command()) == "");

  CHECK(p.command().empty());

  parse_var_name(interp, p, "# dummy");
  CHECK(p.tokens().size() == 1);
  REQUIRE(p.tokens().size() > 0u);
  CHECK(p.tokens()[0].type != TCL_TOKEN_VARIABLE);
}


TEST_CASE("parse-var-name")
{
  tcl::parse_result p{};

  CHECK(parse_var_name(interp, p, "ab \"").has_value());
  CHECK(parse_var_name(interp, p, "$ab \"").has_value());
  REQUIRE(p.tokens().size() > 0u);
  auto tk = p.tokens()[0];
  CHECK(p.tokens()[0].type == TCL_TOKEN_VARIABLE);
  CHECK(std::string(tk.start, tk.size) == "$ab");
  CHECK(parse_var_name(interp, p, "$abcd(xyz)"));
  CHECK(std::string(p.command()) == "");

  CHECK(p.command().empty());

  parse_var_name(interp, p, "# dummy");
  CHECK(p.tokens().size() == 1);
  REQUIRE(p.tokens().size() > 0u);
  CHECK(p.tokens()[0].type != TCL_TOKEN_VARIABLE);
}



TEST_CASE("parse-var")
{

  Tcl_SetVar(interp, "abcd", "42", 0);

  CHECK_THROWS(tcl::parse_var(interp, "ab \"").value());
  CHECK_THROWS(tcl::parse_var(interp, "$ab \"").value());
  auto pv = tcl::parse_var(interp, "$abcd");
  REQUIRE_NOTHROW(pv.value());
  CHECK(pv->first == "$abcd");
  CHECK(pv->second == "42");
}
