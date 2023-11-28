//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <tclbind.hpp>
#include <boost/json.hpp>
#include <boost/json/src.hpp>

struct foo
{
  Tcl_Interp* interp;

  template<typename T>
  auto operator()(const T & val) -> tclbind::object_ptr
  {
    return tclbind::make_object(interp, val);
  }
};

// make a null type
const Tcl_ObjType nullType {
    "null",
    nullptr, nullptr, nullptr, nullptr
};

inline tclbind::object_ptr tag_invoke(const tclbind::convert_tag &, Tcl_Interp*, std::nullptr_t )
{
  auto obj = Tcl_NewObj();
  static char null[5] = "null";
  obj->bytes = null;
  obj->length = 4;
  obj->typePtr = & nullType;
  return obj;
}

inline tclbind::object_ptr tag_invoke(const tclbind::convert_tag &,
                                         Tcl_Interp* interp,
                                         const boost::json::string & str)
{
  return Tcl_NewStringObj(str.data(), str.size());
}

inline tclbind::object_ptr tag_invoke(const tclbind::convert_tag &,
                                         Tcl_Interp* interp,
                                         const boost::json::value & value)
{
  using namespace boost;
  return boost::json::visit(foo{interp}, value);
}

inline bool tag_invoke(
    const tclbind::equivalent_type_tag<boost::json::value> & tag,
    const Tcl_ObjType & type)
{
  using tclbind::equivalent_type_tag;
  namespace json = boost::json;

  if (&type == &nullType)
    return true;

  return tag_invoke(equivalent_type_tag<json::string>{}, type)
      || tag_invoke(equivalent_type_tag<json::object>{}, type)
      || tag_invoke(equivalent_type_tag<json::array>{}, type)
      || tag_invoke(equivalent_type_tag<json::string_view>{}, type)
      || tag_invoke(equivalent_type_tag<std::int64_t>{}, type)
      || tag_invoke(equivalent_type_tag<std::uint64_t>{}, type)
      ;
}


inline std::optional<boost::json::value> tag_invoke(
    tclbind::cast_tag<boost::json::value>,
    Tcl_Interp * interp, boost::intrusive_ptr<Tcl_Obj> val)
{
  using namespace tclbind;
  namespace json = boost::json;
  if (!val || val->typePtr == &nullType)
    return nullptr;

  if (is_equivalent_type<json::object>(val->typePtr))
    return try_cast<json::object>(interp, val.get());

  if (is_equivalent_type<json::array>(val->typePtr))
    return try_cast<json::array>(interp, val.get());

  if (is_equivalent_type<std::int64_t>(val->typePtr))
    return try_cast<std::int64_t>(interp, val.get());

  if (is_equivalent_type<std::uint64_t>(val->typePtr))
    return try_cast<std::uint64_t>(interp, val.get());

  auto ss = tclbind::try_cast_without_implicit_string<json::string>(interp, val.get());
  if (ss)
    return std::move(*ss);

  return std::nullopt;
}


inline std::optional<std::nullptr_t> tag_invoke(
    tclbind::cast_tag<std::nullptr_t>,
    Tcl_Interp * interp, boost::intrusive_ptr<Tcl_Obj> val)
{
  using namespace tclbind;
  namespace json = boost::json;
  if (!val || val->typePtr == &nullType ||
      (val->typePtr == nullptr && val->length == 0))
    return nullptr;

  return std::nullopt;
}

template<typename T>
auto serialize_func = static_cast<std::string(*)(const T &)>(&boost::json::serialize);

TCLBIND_PACKAGE(Json, "1.0", mod)
{
  using namespace boost;
  tclbind::create_command(mod, "json::parse").add_function(
      [](boost::json::string_view sv)
      {
        return boost::json::parse(sv);
      });

  tclbind::set(mod, "json::null", nullptr);
  tclbind::create_command(mod, "json::serialize")
      .add_function(serialize_func<json::value>)
      .add_function(serialize_func<json::array>)
      .add_function(serialize_func<json::object>)
      .add_function(serialize_func<json::string>)
      .add_function(
          [](std::nullptr_t) -> std::string
          {
            return "null";
          })
      .add_function(
          [](boost::core::string_view v)
          {
            return boost::json::serialize(v);
          })
      ;
}