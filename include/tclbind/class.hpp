//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef TCLBIND_CLASS_HPP
#define TCLBIND_CLASS_HPP

#include <tclbind/cast.hpp>
#include <tclbind/command.hpp>
#include <tclbind/detail/overload_traits.hpp>
#include <tclbind/exception.hpp>

#include <tcl.h>
#include <tclOO.h>

#include <boost/core/detail/string_view.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/variadic/to_seq.hpp>
#include <boost/mp11/list.hpp>
#include <boost/mp11/algorithm.hpp>
#include <boost/type_index.hpp>

namespace tclbind
{

namespace detail
{

template<typename T>
struct get_constructors_tag {};

#define TCLBIND_DESCRIBE_CONSTRUCTORS_STEP(r, data, elem) \
        BOOST_PP_COMMA_IF( BOOST_PP_SUB ( r , 1)) data(*) elem

#define TCLBIND_DESCRIBE_CONSTRUCTORS(Type, ...) \
auto tag_invoke(tclbind::detail::get_constructors_tag<Type>) -> \
    boost::mp11::mp_list< BOOST_PP_SEQ_FOR_EACH(TCLBIND_DESCRIBE_CONSTRUCTORS_STEP, Type, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)) >;

template<typename T>
struct get_class_name_tag {};

template<typename T>
auto tag_invoke(tclbind::detail::get_class_name_tag<T>) -> boost::core::string_view
{
  static std::string name = boost::typeindex::type_id<T>().pretty_name();
  return name;
}

#define TCLBIND_SET_CLASS_NAME(Type, Name) \
auto tag_invoke(tclbind::detail::get_class_name_tag<Type>) -> core::string_view {return #Name;}


template<typename T>
static Tcl_ObjectMetadataType thisMetaData {
  TCL_OO_METHOD_VERSION_CURRENT, "this",
  +[](ClientData data)
  {
    delete static_cast<T*>(data);
  },
  +[](Tcl_Interp *interp, ClientData oldClientData, ClientData *newClientData)
  {
    try
    {
      *newClientData = new T(*static_cast<T*>(oldClientData));
      return TCL_OK;
    }
    catch(...)
    {
      auto obj = ::tclbind::make_exception_object();
      Tcl_SetObjResult(interp, obj.get());
      return TCL_ERROR;
    }
  }
};

template<typename T>
static Tcl_ObjectMetadataType typeMetaData {
    TCL_OO_METHOD_VERSION_CURRENT, "this",
    nullptr,
    nullptr
};



struct constructor_call_equal
{
  Tcl_Interp * interp;
  int objc;
  Tcl_Obj * const *objv;
  void* &res;

  template<typename Class, typename ... Args, std::size_t ...Idx>
  void call_impl(Class (*)(Args...), std::index_sequence<Idx...>)
  {
    using types = boost::mp11::mp_list<Args...>;
    const bool all_equal = (is_equal_type<boost::mp11::mp_at_c<types, Idx>>(interp, objv[Idx]) && ...);
    if (all_equal)
        res = new Class(*try_cast<boost::mp11::mp_at_c<types, Idx>>(interp, objv[Idx])...);
  }

  template<typename Class, typename ... Args>
  void operator()(Class (*f)(Args...))
  {
    if ((sizeof...(Args) != objc) || (res != nullptr))
      return;
    call_impl(f, std::make_index_sequence<sizeof...(Args)>{});
  }
};

struct constructor_call_equivalent
{
  Tcl_Interp * interp;
  int objc;
  Tcl_Obj * const *objv;
  void* &res;

  template<typename Class, typename ... Args, std::size_t ...Idx>
  void call_impl(Class (*)(Args...), std::index_sequence<Idx...>)
  {
    using types = boost::mp11::mp_list<Args...>;
    const bool all_equal = (is_equivalent_type<boost::mp11::mp_at_c<types, Idx>>(objv[Idx]->typePtr) && ...);
    if (all_equal)
      res = new Class(*try_cast<boost::mp11::mp_at_c<types, Idx>>(interp, objv[Idx])...);
  }

  template<typename Class, typename ... Args>
  void operator()(Class (*f)(Args...))
  {
    if ((sizeof...(Args) != objc) || (res != nullptr))
      return;
    call_impl(f, std::make_index_sequence<sizeof...(Args)>{});
  }
};


struct constructor_call_castable
{
  Tcl_Interp * interp;
  int objc;
  Tcl_Obj * const *objv;
  void* &res;

  template<typename Class, typename ... Args, std::size_t ...Idx>
  void call_impl(Class (*)(Args...), std::index_sequence<Idx...>)
  {
    using types = boost::mp11::mp_list<Args...>;
    const auto opts = std::make_tuple(
        try_cast_without_implicit_string<boost::mp11::mp_at_c<types, Idx>>(interp, objv[Idx])...
        );

    const bool all_equal = (!!std::get<Idx>(opts) && ...);
    if (all_equal)
      res = new Class(*std::move(std::get<Idx>(opts))...);
  }

  template<typename Class, typename ... Args>
  void operator()(Class (*f)(Args...))
  {
    if ((sizeof...(Args) != objc) || (res != nullptr))
      return;
    call_impl(f, std::make_index_sequence<sizeof...(Args)>{});
  }
};

struct constructor_call_with_string
{
  Tcl_Interp * interp;
  int objc;
  Tcl_Obj * const *objv;
  void* &res;

  template<typename Class, typename ... Args, std::size_t ...Idx>
  void call_impl(Class (*)(Args...), std::index_sequence<Idx...>)
  {
    using types = boost::mp11::mp_list<Args...>;
    const auto opts = std::make_tuple(
        try_cast<boost::mp11::mp_at_c<types, Idx>>(interp, objv[Idx])...
    );

    const bool all_equal = (!!std::get<Idx>(opts) && ...);
    if (all_equal)
      res = new Class(*std::move(std::get<Idx>(opts))...);
  }

  template<typename Class, typename ... Args>
  void operator()(Class (*f)(Args...))
  {
    if ((sizeof...(Args) != objc) || (res != nullptr))
      return;
    call_impl(f, std::make_index_sequence<sizeof...(Args)>{});
  }
};



template<typename T>
int constructor_impl(ClientData clientData, Tcl_Interp *interp,
                     Tcl_ObjectContext objectContext, int objc, Tcl_Obj *const *objv)

try
{

  void* res = nullptr;

  if (objc == 1) // check if internal
  {
    auto obj = objv[0];
    boost::core::string_view nm{obj->bytes, static_cast<std::size_t>(obj->length)};
    if (nm ==  "tclbind::constructor::helper")
    {
      if ((*static_cast<std::type_info*>(obj->internalRep.twoPtrValue.ptr2) != typeid(T)))
      {
        constexpr char msg[] = "mismatched type";
        return TCL_ERROR;
      }
      res = obj->internalRep.twoPtrValue.ptr1;
    }
  }

  using ctors = decltype(tag_invoke(get_constructors_tag<T>{}));
  const int skip = Tcl_ObjectContextSkippedArgs(objectContext);
  objc -= skip;
  objv += skip;
  if (res == nullptr)
    boost::mp11::mp_for_each<ctors>(constructor_call_equal{interp, objc, objv, res});
  if (res == nullptr)
    boost::mp11::mp_for_each<ctors>(constructor_call_equivalent{interp, objc, objv, res});
  if (res == nullptr)
    boost::mp11::mp_for_each<ctors>(constructor_call_castable{interp, objc, objv, res});
  if (res == nullptr)
    boost::mp11::mp_for_each<ctors>(constructor_call_with_string{interp, objc, objv, res});

  if (res)
  {
    auto ctx = Tcl_ObjectContextObject(objectContext);
    Tcl_ObjectSetMetadata(ctx, &thisMetaData<T>, res);
    Tcl_ObjectSetMetadata(ctx, &typeMetaData<T>, const_cast<std::type_info*>(&typeid(res)));
    return TCL_OK;
  }
  constexpr char msg[] = "no constructor";
  Tcl_SetObjResult(interp, Tcl_NewStringObj(msg, sizeof(msg) - 1));
  return TCL_ERROR;
}
catch (...)
{
  auto obj = ::tclbind::make_exception_object();
  Tcl_SetObjResult(interp, obj.get());
  return TCL_ERROR;
}

template<typename T>
const Tcl_MethodType constructorType{
    TCL_OO_METHOD_VERSION_CURRENT,
    "constructor",
    &constructor_impl<T>,
    nullptr,
    nullptr
};

template<typename T>
const Tcl_MethodType destructorType{
    TCL_OO_METHOD_VERSION_CURRENT,
    "destructor",
    [](ClientData clientData, Tcl_Interp *interp,
       Tcl_ObjectContext objectContext, int objc, Tcl_Obj *const *objv)
    {
      auto ctx = Tcl_ObjectContextObject(objectContext);
      auto this_ = Tcl_ObjectGetMetadata(ctx, &thisMetaData<T>);
      if (this_ != nullptr)
        delete static_cast<T*>(this_);
      return TCL_OK;
    },
    nullptr,
    nullptr
};


template<typename T>
struct method_call_equal
{
  Tcl_Interp * interp;
  int objc;
  Tcl_Obj * const *objv;
  bool & done;
  T * this_;
  boost::core::string_view name;

  template<typename Descriptor, typename Types, std::size_t ...Idx>
  void call_impl(const Descriptor & descr, Types *,
                  std::true_type /* is void */, std::index_sequence<Idx...> )
  {
    const bool all_equal = (is_equal_type<boost::mp11::mp_at_c<Types, Idx>>(interp, objv[Idx]) && ...);
    if (all_equal)
    {
      (this_->*Descriptor::pointer)(*try_cast<boost::mp11::mp_at_c<Types, Idx + 1>>(interp, objv[Idx])...);
      done = true;
    }
  }

  template<typename Descriptor, typename Types, std::size_t ...Idx>
  void call_impl(const Descriptor & descr, Types *,
                 std::false_type /* is void */ , std::index_sequence<Idx...> )
  {
    const bool all_equal = (is_equal_type<boost::mp11::mp_at_c<Types, Idx>>(interp, objv[Idx]) && ...);
    if (all_equal)
    {
      auto res = (this_->*Descriptor::pointer)(*try_cast<boost::mp11::mp_at_c<Types, Idx + 1>>(interp, objv[Idx])...);
      auto obj = make_object(interp, std::move(res));
      Tcl_SetObjResult(interp, obj.get());
      done = true;
    }
  }

  template<typename Descriptor>
  void operator()(const Descriptor & descr)
  {
    using args_t = boost::callable_traits::args_t<decltype(Descriptor::pointer)>;
    if (((std::tuple_size<args_t>::value - 1) != objc) || done || (name != Descriptor::name))
      return;
    call_impl(descr, static_cast<args_t*>(nullptr),
              std::is_void<boost::callable_traits::return_type_t<decltype(Descriptor::pointer)>>{},
              std::make_index_sequence<std::tuple_size<args_t>::value - 1>{});
  }
};

template<typename T>
struct method_call_equivalent
{
  Tcl_Interp * interp;
  int objc;
  Tcl_Obj * const *objv;
  bool & done;
  T * this_;
  boost::core::string_view name;

  template<typename Descriptor, typename Types, std::size_t ...Idx>
  void call_impl(const Descriptor & descr, Types *,
                 std::true_type /* is void */ , std::index_sequence<Idx...>)
  {
    const bool all_equal = (is_equivalent_type<boost::mp11::mp_at_c<Types, Idx + 1>>(objv[Idx]->typePtr) && ...);
    if (all_equal)
    {
      (this_->*Descriptor::pointer)(*try_cast<boost::mp11::mp_at_c<Types, Idx + 1>>(interp, objv[Idx])...);
      done = true;
    }
  }

  template<typename Descriptor, typename Types, std::size_t ...Idx>
  void call_impl(const Descriptor & descr, Types *,
                 std::false_type /* is void */ , std::index_sequence<Idx...>)
  {
    const bool all_equal = (is_equivalent_type<boost::mp11::mp_at_c<Types, Idx + 1>>(objv[Idx]->typePtr) && ...);
    if (all_equal)
    {
      auto res = (this_->*Descriptor::pointer)(*try_cast<boost::mp11::mp_at_c<Types, Idx + 1>>(interp, objv[Idx])...);
      auto obj = make_object(interp, std::move(res));
      Tcl_SetObjResult(interp, obj.get());
      done = true;
    }
  }


  template<typename Descriptor>
  void operator()(const Descriptor & descr)
  {
    using args_t = boost::callable_traits::args_t<decltype(Descriptor::pointer)>;
    if (((std::tuple_size<args_t>::value - 1) != objc) || done || (name != Descriptor::name))
      return;
    call_impl(descr, static_cast<args_t*>(nullptr),
              std::is_void<boost::callable_traits::return_type_t<decltype(Descriptor::pointer)>>{},
              std::make_index_sequence<std::tuple_size<args_t>::value - 1>{});
  }
};


template<typename T>
struct method_call_castable
{
  Tcl_Interp * interp;
  int objc;
  Tcl_Obj * const *objv;
  bool & done;
  T * this_;
  boost::core::string_view name;

  template<typename Descriptor, typename Types, std::size_t ...Idx>
  void call_impl(const Descriptor & descr, Types *,
                 std::true_type /* is void */, std::index_sequence<Idx...> )
  {
    const auto opts = std::make_tuple(
        try_cast_without_implicit_string<boost::mp11::mp_at_c<Types, Idx + 1>>(interp, objv[Idx])...
    );

    const bool all_equal = (!!std::get<Idx>(opts) && ...);
    if (all_equal)
    {
      (this_->*Descriptor::pointer)(*std::move(std::get<Idx>(opts))...);
      done = true;
    }
  }

  template<typename Descriptor, typename Types, std::size_t ...Idx>
  void call_impl(const Descriptor & descr, Types *,
                 std::false_type /* is void */ , std::index_sequence<Idx...>)
  {
    const auto opts = std::make_tuple(
        try_cast_without_implicit_string<boost::mp11::mp_at_c<Types, Idx + 1>>(interp, objv[Idx])...
    );

    const bool all_equal = (!!std::get<Idx>(opts) && ...);
    if (all_equal)
    {
      auto res = (this_->*Descriptor::pointer)(*std::move(std::get<Idx>(opts))...);
      auto obj = make_object(interp, std::move(res));
      Tcl_SetObjResult(interp, obj.get());
      done = true;
    }
  }

  template<typename Descriptor>
  void operator()(const Descriptor & descr)
  {
    using args_t = boost::callable_traits::args_t<decltype(Descriptor::pointer)>;
    if (((std::tuple_size<args_t>::value - 1) != objc) || done || (name != Descriptor::name))
      return;

    call_impl(descr, static_cast<args_t*>(nullptr),
              std::is_void<boost::callable_traits::return_type_t<decltype(Descriptor::pointer)>>{},
              std::make_index_sequence<std::tuple_size<args_t>::value - 1>{});
  }
};

template<typename T>
struct method_call_with_string
{
  Tcl_Interp * interp;
  int objc;
  Tcl_Obj * const *objv;
  bool & done;
  T * this_;
  boost::core::string_view name;

  template<typename Descriptor, typename Types, std::size_t ...Idx>
  void call_impl(const Descriptor & descr, Types *,
                 std::true_type /* is void */ , std::index_sequence<Idx...>)
  {
    const auto opts = std::make_tuple(
        try_cast<boost::mp11::mp_at_c<Types, Idx + 1>>(interp, objv[Idx])...
    );

    const bool all_equal = (!!std::get<Idx>(opts) && ...);
    if (all_equal)
    {
      (this_->*Descriptor::pointer)(*std::move(std::get<Idx>(opts))...);
      done = true;
    }
  }

  template<typename Descriptor, typename Types, std::size_t ...Idx>
  void call_impl(const Descriptor & descr, Types *,
                 std::false_type /* is void */, std::index_sequence<Idx...> )
  {
    const auto opts = std::make_tuple(
        try_cast<boost::mp11::mp_at_c<Types, Idx + 1>>(interp, objv[Idx])...
    );

    const bool all_equal = (!!std::get<Idx>(opts) && ...);
    if (all_equal)
    {
      auto res = (this_->*Descriptor::pointer)(*std::move(std::get<Idx>(opts))...);
      auto obj = make_object(interp, std::move(res));
      Tcl_SetObjResult(interp, obj.get());
      done = true;
    }
  }
  template<typename Descriptor>
  void operator()(const Descriptor & descr)
  {
    using args_t = boost::callable_traits::args_t<decltype(Descriptor::pointer)>;
    if (((std::tuple_size<args_t>::value - 1) != objc) || done || (name != Descriptor::name))
      return;

    call_impl(descr, static_cast<args_t*>(nullptr),
              std::is_void<boost::callable_traits::return_type_t<decltype(Descriptor::pointer)>>{},
              std::make_index_sequence<std::tuple_size<args_t>::value - 1>{});
  }
};

template<typename T>
int method_impl(ClientData clientData, Tcl_Interp *interp,
                Tcl_ObjectContext objectContext, int objc, Tcl_Obj *const *objv)
try
{
  using descriptor = boost::describe::describe_members<T,
                      boost::describe::mod_inherited | boost::describe::mod_function | boost::describe::mod_public >;

  auto ctx = Tcl_ObjectContextObject(objectContext);
  auto this_ = static_cast<T*>(Tcl_ObjectGetMetadata(ctx, &thisMetaData<T>));
  assert(this_ != nullptr);
  const int skip = Tcl_ObjectContextSkippedArgs(objectContext);

  objc -= skip;
  objv += skip;

  const auto meth = Tcl_MethodName(Tcl_ObjectContextMethod(objectContext));
  const boost::core::string_view name{meth->bytes, static_cast<std::size_t>(meth->length)};

  bool done = false;
  boost::mp11::mp_for_each<descriptor>(method_call_equal<T   >     {interp, objc, objv, done, this_, name});
  if (!done)
    boost::mp11::mp_for_each<descriptor>(method_call_equivalent<T> {interp, objc, objv, done, this_, name});
  if (!done)
    boost::mp11::mp_for_each<descriptor>(method_call_castable<T>   {interp, objc, objv, done, this_, name});
  if (!done)
    boost::mp11::mp_for_each<descriptor>(method_call_with_string<T>{interp, objc, objv, done, this_, name});

  if (done)
    return TCL_OK;

  constexpr char msg[] = "no matching method overload";
  Tcl_SetObjResult(interp, Tcl_NewStringObj(msg, sizeof(msg) - 1));
  return TCL_ERROR;
}
catch (...)
{
  auto obj = ::tclbind::make_exception_object();
  Tcl_SetObjResult(interp, obj.get());
  return TCL_ERROR;
}

template<typename T>
const Tcl_MethodType & getMethodType(const char * name)
{
  static const Tcl_MethodType methodType{
      TCL_OO_METHOD_VERSION_CURRENT,
      name,
      &method_impl<T>,
      nullptr,
      nullptr
  };

  return methodType;
}

template<typename T>
int static_method_impl(ClientData clientData, Tcl_Interp *interp,
                Tcl_ObjectContext objectContext, int objc, Tcl_Obj *const *objv)
try
{
  const int skip = Tcl_ObjectContextSkippedArgs(objectContext);

  objc -= skip;
  objv += skip;

  objc ++;
  objv --;

  const auto meth = Tcl_MethodName(Tcl_ObjectContextMethod(objectContext));
  const boost::core::string_view name{meth->bytes, static_cast<std::size_t>(meth->length)};

  using descriptors = boost::describe::describe_members<T,
                    boost::describe::mod_inherited | boost::describe::mod_function |
                    boost::describe::mod_public | boost::describe::mod_static >;

  int res = TCL_CONTINUE;
  boost::mp11::mp_for_each<descriptors>(
      [&](auto descr)
      {
        if (res != TCL_CONTINUE && descr.name == name)
          res = overload_traits<decltype(descr.pointer)>::call_equal(descr.pointer, interp, objc, objv);
      });
  if (res != TCL_CONTINUE)
    boost::mp11::mp_for_each<descriptors>(
          [&](auto descr)
            {
              if (res != TCL_CONTINUE && descr.name == name)
                res = overload_traits<decltype(descr.pointer)>::call_equivalent(descr.pointer, interp, objc, objv);
            });
  if (res != TCL_CONTINUE)
    boost::mp11::mp_for_each<descriptors>(
          [&](auto descr)
            {
              if (res != TCL_CONTINUE && descr.name == name)
                res = overload_traits<decltype(descr.pointer)>::call_castable(descr.pointer, interp, objc, objv);
            });
  if (res != TCL_CONTINUE)
    boost::mp11::mp_for_each<descriptors>(
        [&](auto descr)
        {
          if (res != TCL_CONTINUE && descr.name == name)
            res = overload_traits<decltype(descr.pointer)>::call_with_string(descr.pointer, interp, objc, objv);
        });

  if (res == TCL_CONTINUE)
    return TCL_OK;

  constexpr char msg[] = "no method";
  Tcl_SetObjResult(interp, Tcl_NewStringObj(msg, sizeof(msg) - 1));
  return TCL_ERROR;
}
catch (...)
{
  auto obj = ::tclbind::make_exception_object();
  Tcl_SetObjResult(interp, obj.get());
  return TCL_ERROR;
}

template<typename T>
const Tcl_MethodType & getStaticMethodType(const char * name)
{
  static const Tcl_MethodType staticMethodType{
      TCL_OO_METHOD_VERSION_CURRENT,
      name,
      &static_method_impl<T>,
      nullptr,
      nullptr
  };
  return staticMethodType;
}

}


template<typename T>
Tcl_Class register_class(Tcl_Interp * interp)
{
  auto cl_name = tag_invoke(detail::get_class_name_tag<T>{});
  object_ptr className = Tcl_NewStringObj(cl_name.data(), cl_name.size());

  Tcl_Object existing = Tcl_GetObjectFromObj(interp, className.get());
  if (existing != nullptr)
    return Tcl_GetObjectAsClass(existing);

  auto classobj = Tcl_GetObjectFromObj(interp, Tcl_NewStringObj("::oo::class" ,-1));
  auto class_ = Tcl_GetObjectAsClass(classobj);
  auto o = Tcl_NewObjectInstance(interp, class_, cl_name.data(), nullptr ,0 ,nullptr ,0);
  auto cl = Tcl_GetObjectAsClass(o);

  auto ctor = Tcl_NewInstanceMethod(interp, o, nullptr, 1, &detail::constructorType<T>, nullptr);
  Tcl_ClassSetConstructor(interp, cl, ctor);

  auto dtor = Tcl_NewInstanceMethod(interp, o, nullptr, 1, &detail::destructorType<T>, nullptr);
  Tcl_ClassSetDestructor(interp, cl, dtor);

  using methods = boost::describe::describe_members<T,
      boost::describe::mod_inherited | boost::describe::mod_function | boost::describe::mod_public >;

  std::array<const char*, boost::mp11::mp_size<methods>::value> method_names;
  auto itr = method_names.begin();

  boost::mp11::mp_for_each<methods>(
      [&](auto desc)
      {
        auto it = std::find_if(
            method_names.begin(), itr,
            [&](const char * nm)
            {
               return std::strcmp(nm, desc.name) == 0;
            });

        if (it == itr)
        {
          *it = desc.name;
          Tcl_NewMethod(interp, cl,
                        Tcl_NewStringObj(desc.name, -1), 1,
                        &detail::getMethodType<T>(desc.name),
                        nullptr);
        }
      });


  using static_methods = boost::describe::describe_members<T,
      boost::describe::mod_inherited | boost::describe::mod_function |
      boost::describe::mod_public | boost::describe::mod_static >;

  std::array<const char*, boost::mp11::mp_size<static_methods>::value> static_method_names;
  itr = static_method_names.begin();
  boost::mp11::mp_for_each<static_methods>(
      [&](auto desc)
      {
        auto it = std::find_if(
            static_method_names.begin(), itr,
            [&](const char * nm)
            {
              return std::strcmp(nm, desc.name) == 0;
            });
        if (it == itr)
        {
          *it = desc.name;
          Tcl_NewInstanceMethod(interp, o,
                                Tcl_NewStringObj(desc.name, -1), 1,
                                &detail::getStaticMethodType<T>(desc.name), nullptr);
        }
      });
  return cl;
}

template<typename T>
auto tag_invoke(
      cast_tag<T>,
      Tcl_Interp * interp,
      boost::intrusive_ptr<Tcl_Obj> val)
      -> std::enable_if_t<boost::describe::has_describe_members<T>::value, T> *
{

  auto obj = Tcl_GetObjectFromObj(interp, val.get());
  if (obj == nullptr)
    return nullptr;

  auto tt = Tcl_ObjectGetMetadata(obj, &detail::typeMetaData<T>);
  if (!tt || (*static_cast<std::type_info*>(tt) == typeid(T)))
    return nullptr;

  tt = Tcl_ObjectGetMetadata(obj, &detail::thisMetaData<T>);
  return static_cast<T*>(tt);
}


template<typename T>
auto tag_invoke(
    equal_type_tag<T>,
    Tcl_Interp * interp,
    Tcl_Obj * val)
    -> std::enable_if_t<boost::describe::has_describe_members<T>::value, bool>
{

  auto obj = Tcl_GetObjectFromObj(interp, val);
  if (obj == nullptr)
    return nullptr;

  auto tt = Tcl_ObjectGetMetadata(obj, &detail::typeMetaData<T>);
  return tt && (*static_cast<std::type_info*>(tt) == typeid(T));
}

template<typename T>
inline auto tag_invoke(const convert_tag &, Tcl_Interp* interp, T && t)
    -> std::enable_if_t<boost::describe::has_describe_members<T>::value, object_ptr>
{
  auto cl = register_class<std::decay_t<T>>(interp);

  auto cl_name = tag_invoke(detail::get_class_name_tag<T>{});
  static char internalConstructorMarker[] = "tclbind::constructor::helper";
  Tcl_Obj objv[1] = {
      0, internalConstructorMarker,
      sizeof(internalConstructorMarker) - 1,
      nullptr
  };

  using type = std::decay_t<T>;
  std::unique_ptr<type> ptr{std::make_unique<type>(std::forward<T>(t))};

  objv->internalRep.twoPtrValue.ptr1 = ptr.get();
  objv->internalRep.twoPtrValue.ptr2 = const_cast<std::type_info*>(&typeid(T));

  auto objp = &objv[0];
  auto obj = Tcl_NewObjectInstance(interp, cl, nullptr, nullptr, 1, &objp, 0);
  if (obj != nullptr)
  {
    ptr.release();
    return Tcl_GetObjectName(interp, obj);
  }
  else
    return nullptr;

}



}

#endif //TCLBIND_CLASS_HPP
