//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_TCL_PARSE_HPP
#define BOOST_TCL_PARSE_HPP

#include <tcl.h>

#include <boost/tcl/interpreter.hpp>
#include <boost/tcl/exception.hpp>
#include <boost/tcl/object.hpp>

#include <boost/core/detail/string_view.hpp>
#include <boost/core/span.hpp>

#include <optional>

namespace boost::tcl
{

struct parse_result : Tcl_Parse
{
  parse_result() = default;
  parse_result(const parse_result & ) noexcept = default;
  parse_result(parse_result && lhs) noexcept : Tcl_Parse{lhs}
  {
    lhs.tokenPtr = lhs.staticTokens;
  }

  ~parse_result()
  {
    Tcl_FreeParse(this);
  }

  core::string_view comment() const {return core::string_view{commentStart, static_cast<std::size_t>(commentSize)};}
  core::string_view command() const {return core::string_view{commandStart, static_cast<std::size_t>(commandSize)};}

  span<Tcl_Token> tokens() const {return span<Tcl_Token>{tokenPtr, static_cast<std::size_t>(numTokens)};}

  int error_type() { return errorType; }

};

result<void> parse_command(Tcl_Interp * interp, parse_result & parse, core::string_view cmd, bool nested = false)
{
  auto cd = Tcl_ParseCommand(interp, cmd.data(), cmd.size(), nested ? 1 : 0, &parse);
  if (cd != TCL_OK)
    return result<void>{system::in_place_error, Tcl_GetObjResult(interp)};

  return system::in_place_value;
}

result<void> parse_expr(Tcl_Interp * interp, parse_result & parse, core::string_view cmd)
{
  auto cd = Tcl_ParseExpr(interp, cmd.data(), cmd.size(), &parse);
  if (cd != TCL_OK)
    return result<void>{system::in_place_error, Tcl_GetObjResult(interp)};

  return system::in_place_value;
}

result<core::string_view> parse_braces(Tcl_Interp * interp, parse_result & parse,
                                       core::string_view cmd, bool append = false)
{
  const char * term;
  auto cd = Tcl_ParseBraces(interp, cmd.data(), cmd.size(), &parse, append ? 1 : 0, & term);
  if (cd != TCL_OK)
    return result<core::string_view>{system::in_place_error, Tcl_GetObjResult(interp)};

  return {system::in_place_value, core::string_view{cmd.data(), term}};
}


result<core::string_view> parse_quoted_string(Tcl_Interp * interp, parse_result & parse,
                                       core::string_view cmd, bool append = false)
{
  const char * term;
  auto cd = Tcl_ParseQuotedString(interp, cmd.data(), cmd.size(), &parse, append ? 1 : 0, & term);
  if (cd != TCL_OK)
    return result<core::string_view>{system::in_place_error, Tcl_GetObjResult(interp)};

  return {system::in_place_value, core::string_view{cmd.data(), term}};
}

result<void> parse_var_name(Tcl_Interp * interp, parse_result & parse,
                            core::string_view cmd, bool append = false)
{
  auto cd = Tcl_ParseVarName(interp, cmd.data(), cmd.size(), &parse, append ? 1 : 0);
  if (cd != TCL_OK)
    return result<void>{system::in_place_error, Tcl_GetObjResult(interp)};

  return system::in_place_value;
}

result<std::pair<core::string_view, core::string_view>> parse_var(Tcl_Interp * interp,
                                    const char * var)
{
  using result_t = result<std::pair<core::string_view, core::string_view>>;
  const char * term;
  auto r = Tcl_ParseVar(interp, var, &term);
  if (r == nullptr)
    return result_t{system::in_place_error, Tcl_GetObjResult(interp)};
  else
    return result_t{core::string_view(var, term), r};
}


result<void> parse_command(const interpreter_ptr & interp, parse_result & parse, core::string_view cmd, bool nested = false)
{
  return parse_command(interp.get(), parse, cmd, nested);

}

result<void> parse_expr(const interpreter_ptr & interp, parse_result & parse, core::string_view cmd)
{
  return parse_expr(interp.get(), parse, cmd);

}

result<core::string_view> parse_braces(const interpreter_ptr & interp, parse_result & parse,
                                       core::string_view cmd, bool append = false)
{
  return parse_braces(interp.get(), parse, cmd, append);
}


result<core::string_view> parse_quoted_string(const interpreter_ptr & interp, parse_result & parse,
                                              core::string_view cmd, bool append = false)
{
  return parse_quoted_string(interp.get(), parse, cmd, append);
}

result<void> parse_var_name(const interpreter_ptr & interp, parse_result & parse,
                            core::string_view cmd, bool append = false)
{
  return parse_var_name(interp.get(), parse, cmd, append);
}

result<std::pair<core::string_view, core::string_view>> parse_var(const interpreter_ptr & interp,
                                                                  const char * var)
{
  return parse_var(interp.get(), var);
}


}

#endif //BOOST_TCL_PARSE_HPP