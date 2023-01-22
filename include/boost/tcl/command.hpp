//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_TCL_COMMAND_HPP
#define BOOST_TCL_COMMAND_HPP

#include <tcl.h>
#include <boost/core/span.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/system/error_code.hpp>
#include <boost/mp11/algorithm.hpp>
#include <boost/describe.hpp>
#include <boost/describe/members.hpp>
#include <boost/unordered_map.hpp>

#include <boost/tcl/class.hpp>
#include <boost/tcl/exception.hpp>
#include <boost/tcl/enum.hpp>

namespace boost::tcl
{


namespace detail
{

template<typename Func>
struct overload_traits;

template<typename Return, typename ... Args>
struct overload_traits<Return(Args...)>
{
    constexpr static std::size_t cnt = sizeof...(Args);
    constexpr static std::make_index_sequence<cnt> seq{};
    using function_type = Return(*)(Args...);

    template<std::size_t Idx>
    using type = std::tuple_element_t<Idx, std::tuple<Args...>>;

    template<typename ... Opts>
    static int try_invoke_no_string_impl(function_type func,
                                         std::enable_if_t<!std::is_void_v<decltype(func(*std::declval<Opts>()...))>,  Tcl_Interp> * interp,
                                         Opts && ... opts)
    {
        const bool invocable = (!!opts && ...);
        if (invocable)
        {
            auto res = func(*std::move(opts)...);
            auto obj = make_object(interp, std::move(res));
            Tcl_SetObjResult(interp, obj.get());
            return TCL_OK;
        }
        else
            return TCL_CONTINUE;
    }

    template<typename ... Opts>
    static int try_invoke_no_string_impl(function_type func,
                                         std::enable_if_t<std::is_void_v<decltype(func(*std::declval<Opts>()...))>, Tcl_Interp>  * interp,
                                         Opts && ... opts)
    {
        const bool invocable = (!!opts && ...);
        if (invocable)
        {
            func(*std::move(opts)...);
            return TCL_OK;
        }
        else
            return TCL_CONTINUE;
    }

    template<std::size_t ... Idx>
    static int try_invoke_no_string(function_type func, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[],
                                    std::index_sequence<Idx...> seq = {})
    {
        return try_invoke_no_string_impl(
                func, interp,
                // if you get an error you're missing some tag_invokes.
                try_cast_without_implicit_string<type<Idx>>(interp, objv[Idx + 1])...);
    }


    template<typename ... Opts>
    static int invoke_impl(function_type func,
                           std::enable_if_t<!std::is_void_v<decltype(func(*std::declval<Opts>()...))>, Tcl_Interp>  * interp,
                           Opts && ... opts)
    {
        const bool invocable = (!!opts && ...);
        if (invocable)
        {
            auto res = func(*std::move(opts)...);
            auto obj = make_object(interp, std::move(res));
            Tcl_SetObjResult(interp, obj.get());
            return TCL_OK;
        }
        else
            return TCL_CONTINUE;
    }

    template<typename ... Opts>
    static int invoke_impl(function_type func,
                           std::enable_if_t<std::is_void_v<decltype(func(*std::declval<Opts>()...))>, Tcl_Interp>  * interp,
                           Opts && ... opts)
    {
        const bool invocable = (!!opts && ...);
        if (invocable)
        {
            func(*std::move(opts)...);
            return TCL_OK;
        }
        else
            return TCL_CONTINUE;
    }


    template<std::size_t ... Idx>
    static int invoke(function_type func, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[],
                      std::index_sequence<Idx...> seq = {})
    {
        return invoke_impl(
                func, interp,
                try_cast<type<Idx>>(interp, objv[Idx + 1])...);
    }

    template<std::size_t ... Idx>
    static int call_equal_impl(function_type func, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[],
                               std::index_sequence<Idx...> seq = {})
    {
        const bool all_equal = (is_equal_type<type<Idx>>(objv[Idx + 1]->typePtr) && ... );
        if (all_equal)
            return invoke(func, interp, objc, objv, seq);
        else
            return TCL_CONTINUE;
    }

    template<typename Func>
    static int call_equal(Func * func, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
    {
        const auto p = reinterpret_cast<function_type>(func);
        assert(objc == cnt + 1);
        return call_equal_impl(p, interp, objc, objv, seq);
    }

    template<std::size_t ... Idx>
    static int call_equivalent_impl(function_type func, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[],
                        std::index_sequence<Idx...> seq = {})
    {
        const bool all_equal = (is_equivalent_type<type<Idx>>(objv[Idx + 1]->typePtr) && ... );
        if (all_equal)
            return invoke(func, interp, objc, objv, seq);
        else
            return TCL_CONTINUE;
    }

    template<typename Func>
    static int call_equivalent(Func * func, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
    {
        const auto p = reinterpret_cast<function_type>(func);
        assert(objc == cnt + 1);
        return call_equivalent_impl(p, interp, objc, objv, seq);
    }

    template<typename Func>
    static int call_castable(Func * func, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
    {
        const auto p = reinterpret_cast<function_type>(func);
        assert(objc == cnt + 1);
        return try_invoke_no_string(p, interp, objc, objv, seq);
    }

    template<typename Func>
    static int call_with_string(Func * func, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
    {
        const auto p = reinterpret_cast<function_type>(func);
        assert(objc == cnt + 1);
        return invoke(p, interp, objc, objv, seq);
    }
};

// same as above, but with interpreter

template<typename Func>
struct overload_traits_with_interp;

template<typename Return, typename Interpreter, typename ... Args>
struct overload_traits_with_interp<Return(Interpreter, Args...)>
{
    constexpr static std::size_t cnt = sizeof...(Args);
    constexpr static std::make_index_sequence<cnt> seq{};
    using function_type = Return(*)(Interpreter, Args...);

    template<std::size_t Idx>
    using type = std::tuple_element_t<Idx, std::tuple<Args...>>;

    template<typename ... Opts>
    static int try_invoke_no_string_impl(function_type func,
                                         std::enable_if_t<!std::is_void_v<decltype(func(std::declval<Tcl_Interp *>(), *std::declval<Opts>()...))>,  Tcl_Interp> * interp,
                                         Opts && ... opts)
    {
        const bool invocable = (!!opts && ...);
        if (invocable)
        {
            auto res = func(interp, *std::move(opts)...);
            auto obj = make_object(interp, std::move(res));
            Tcl_SetObjResult(interp, obj.get());
            return TCL_OK;
        }
        else
            return TCL_CONTINUE;
    }

    template<typename ... Opts>
    static int try_invoke_no_string_impl(function_type func,
                                         std::enable_if_t<std::is_void_v<decltype(func(std::declval<Tcl_Interp *>(), *std::declval<Opts>()...))>, Tcl_Interp>  * interp,
                                         Opts && ... opts)
    {
        const bool invocable = (!!opts && ...);
        if (invocable)
        {
            func(interp, *std::move(opts)...);
            return TCL_OK;
        }
        else
            return TCL_CONTINUE;
    }

    template<std::size_t ... Idx>
    static int try_invoke_no_string(function_type func, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[],
                                    std::index_sequence<Idx...> seq = {})
    {
        return try_invoke_no_string_impl(
                func, interp,
                // if you get an error you're missing some tag_invokes.
                try_cast_without_implicit_string<type<Idx>>(interp, objv[Idx + 1])...);
    }

    template<typename ... Opts>
    static int invoke_impl(function_type func,
                           std::enable_if_t<!std::is_void_v<decltype(func(std::declval<Tcl_Interp *>(), *std::declval<Opts>()...))>, Tcl_Interp>  * interp,
                           Opts && ... opts)
    {
        const bool invocable = (!!opts && ...);
        if (invocable)
        {
            auto res = func(interp, *std::move(opts)...);
            auto obj = make_object(interp, std::move(res));
            Tcl_SetObjResult(interp, obj.get());
            return TCL_OK;
        }
        else
            return TCL_CONTINUE;
    }

    template<typename ... Opts>
    static int invoke_impl(function_type func,
                           std::enable_if_t<std::is_void_v<decltype(func(std::declval<Tcl_Interp *>(), *std::declval<Opts>()...))>, Tcl_Interp>  * interp,
                           Opts && ... opts)
    {
        const bool invocable = (!!opts && ...);
        if (invocable)
        {
            func(interp, *std::move(opts)...);
            return TCL_OK;
        }
        else
            return TCL_CONTINUE;
    }


    template<std::size_t ... Idx>
    static int invoke(function_type func, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[],
                      std::index_sequence<Idx...> seq = {})
    {
        return invoke_impl(
                func, interp,
                try_cast<type<Idx>>(interp, objv[Idx + 1])...);
    }

    template<std::size_t ... Idx>
    static int call_equal_impl(function_type func, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[],
                               std::index_sequence<Idx...> seq = {})
    {
        const bool all_equal = (is_equal_type<type<Idx>>(objv[Idx + 1]->typePtr) && ... );
        if (all_equal)
            return invoke(func, interp, objc, objv, seq);
        else
            return TCL_CONTINUE;
    }

    static int call_equal(void * func, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
    {
        const auto p = reinterpret_cast<function_type>(func);
        assert(objc == cnt + 1);
        return call_equal_impl(p, interp, objc, objv, seq);
    }

    template<std::size_t ... Idx>
    static int call_equivalent_impl(function_type func, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[],
                                    std::index_sequence<Idx...> seq = {})
    {
        const bool all_equal = (is_equivalent_type<type<Idx>>(objv[Idx + 1]->typePtr) && ... );
        if (all_equal)
            return invoke(func, interp, objc, objv, seq);
        else
            return TCL_CONTINUE;
    }

    static int call_equivalent(void * func, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
    {
        const auto p = reinterpret_cast<function_type>(func);
        assert(objc == cnt + 1);
        return call_equivalent_impl(p, interp, objc, objv, seq);
    }
    static int call_castable(void * func, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
    {
        const auto p = reinterpret_cast<function_type>(func);
        assert(objc == cnt + 1);
        return try_invoke_no_string(p, interp, objc, objv, seq);
    }

    static int call_with_string(void * func, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
    {
        const auto p = reinterpret_cast<function_type>(func);
        assert(objc == cnt + 1);
        return invoke(p, interp, objc, objv, seq);
    }
};

}

struct sub_command
{

    template<typename ... Args, typename Return>
    sub_command & add_function(Return (*f)(Args...))
    {
        add_impl_(f);
        return *this;
    }

    template<typename ... Args, typename Return>
    sub_command & add_function_with_interp(Return (*f)(Args...))
    {
        add_with_interp_impl_(f);
        return *this;
    }

    template<typename Signature>
    sub_command & add_constructor()
    {
        return add_constructor_impl_(static_cast<Signature*>(nullptr));
    }
    template<typename Class>
    inline sub_command & add_class()
    {
        if constexpr(std::is_default_constructible_v<Class>)
            add_constructor<Class()>();
        if constexpr(std::is_copy_constructible_v<Class>)
            add_constructor<Class(const Class &)>();

        mp11::mp_for_each<describe::describe_members<Class, describe::mod_static |
                                                            describe::mod_public>>(
            [this](auto t)
            {
               add_subcommand(".get").add_subcommand(t.name).add_function(+[]{return *decltype(t)::pointer;});
               add_subcommand(".set").add_subcommand(t.name).add_function(
                       +[](std::decay_t<decltype(*decltype(t)::pointer)> val)
                       {
                           return *decltype(t)::pointer = std::move(val);
                       });
            });

        mp11::mp_for_each<describe::describe_members<Class, describe::mod_static |
                                                            describe::mod_public |
                                                            describe::mod_function>>(
                [&](auto t)
                {
                    add_subcommand(t.name).add_function(t.pointer);
                });

        return *this;
    }

    template<typename E>
    sub_command & add_enum()
    {
        add_function(
                +[](Tcl_WideInt wi)
                {
                    return static_cast<E>(wi);
                });
        add_function(
                +[](string_view str)
                {
                    std::optional<E> res;
                    boost::mp11::mp_for_each<boost::describe::describe_enumerators<E>>(
                            [&](auto desc )
                            {
                                if (desc.name == str)
                                    res = desc.value;
                            });
                    if (!res)
                        throw std::logic_error("Invalid input for enum: " + std::string(str));
                    return *res;
                });
        return *this;
    }

    sub_command & add_subcommand(string_view sv)
    {
        return sub_commands_[sv.to_string()];
    }

    struct overload_t
    {
        const std::size_t cnt;

        using call_t = int(void*, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[]);
        call_t * call_equal_,
                * call_equivalent_,
                * call_castable_,
                * call_string_;
        void * impl_;

        int call_equal(Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
        {
            return call_equal_(impl_, interp, objc, objv);
        }
        int call_equivalent(Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
        {
            return call_equivalent_(impl_, interp, objc, objv);
        }
        int call_castable(Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
        {
            return call_castable_(impl_, interp, objc, objv);
        }
        int call_with_string(Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
        {
            return call_string_(impl_, interp, objc, objv);
        }
    };
    ~sub_command()
    {

    }
  private:

    template<typename Class, typename ... Args>
    sub_command & add_constructor_impl_(Class(*)(Args...))
    {
        add_subcommand("new").add_function(
                +[](Args ... args)
                {
                    return new Class(static_cast<Args>(args)...);
                });

        return *this;
    }


    template<typename Func>
    void add_impl_(Func *f)
    {
        using traits = detail::overload_traits<Func>;
        overload_t ovl{traits::cnt,
                       &traits::call_equal,
                       &traits::call_equivalent,
                       &traits::call_castable,
                       &traits::call_with_string,
                       reinterpret_cast<void*>(f)};
        overloads_.push_back(ovl);
    }

    template<typename Func>
    void add_with_interp_impl_(Func *f)
    {
        using traits = detail::overload_traits_with_interp<Func>;
        overload_t ovl{traits::cnt,
                       &traits::call_equal,
                       &traits::call_equivalent,
                       &traits::call_castable,
                       &traits::call_with_string,
                       reinterpret_cast<void*>(f)};
        overloads_.push_back(ovl);
    }

  protected:
    int invoke_(Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
    {
        if (objc > 1 &&
            (!objv[1]->typePtr ||
              objv[1]->typePtr->name == string_view("string")))
        {
            auto itr = sub_commands_.find(std::string(objv[1]->bytes, objv[1]->length));
            if (itr != sub_commands_.end())
                return itr->second.invoke_(interp, objc - 1, objv + 1);
        }
        try {
            if (overloads_.size() == 1u) {
                if ((overloads_.front().cnt + 1) != objc)
                    goto err;
                return overloads_.front().call_with_string(interp, objc, objv);
            }
            {
                auto candidates = adaptors::filter(overloads_,
                                                   [&](const overload_t &ovl) { return ovl.cnt == (objc - 1); });

                for (auto can: candidates) // equal match
                    if (auto res = can.call_equal(interp, objc, objv); res != TCL_CONTINUE)
                        return res;

                for (auto can: candidates) // equivalent match
                    if (auto res = can.call_equivalent(interp, objc, objv); res != TCL_CONTINUE)
                        return res;

                for (auto can: candidates) // castable match
                    if (auto res = can.call_castable(interp, objc, objv); res != TCL_CONTINUE)
                        return res;

                for (auto can: candidates) // string match
                    if (auto res = can.call_with_string(interp, objc, objv); res != TCL_CONTINUE)
                        return res;
            }
          err:
            constexpr char msg[] = "no matching overload";
            object_ptr obj = Tcl_NewStringObj(msg, sizeof(msg) - 1);
            Tcl_SetObjResult(interp, obj.get());
            return TCL_ERROR;
        }
        catch (...) {
            auto obj = ::boost::tcl::make_exception_object();
            Tcl_SetObjResult(interp, obj.get());
            return TCL_ERROR;
        }
    }
  private:
    std::vector<overload_t> overloads_;
    boost::unordered_map<std::string, sub_command> sub_commands_;
};

struct command : sub_command
{
    friend command & create_command(Tcl_Interp *interp, const char * name);

    Tcl_CmdInfo info(system::error_code & ec)
    {
        Tcl_CmdInfo info_;
        Tcl_GetCommandInfoFromToken(cmd_, &info_);
        return info_;
    }
    Tcl_Command cmd() {return cmd_;}

  private:

    explicit command() {}
    Tcl_Command cmd_ = nullptr;
};

inline command & create_command(Tcl_Interp *interp, const char * name)
{
    auto cd = new command;
    cd->cmd_ = Tcl_CreateObjCommand(interp, name,
                         +[](ClientData cdata, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
                         {
                            return static_cast<command*>(cdata)->invoke_(interp, objc, objv);
                         }, cd,
                         +[](ClientData cdata) { delete static_cast<command*>(cdata); });
    return *cd;
}

namespace detail
{

template<typename T, describe::modifiers modifiers = describe::mod_public, typename Func>
void enumerate_members(Func && func)
{
    mp11::mp_for_each<describe::describe_members<T, modifiers>>(func);
    mp11::mp_for_each<describe::describe_bases<T,   describe::mod_public>>(
            [&](auto b)
            {
                return enumerate_members<typename decltype(b)::type, modifiers>(func);
            });
}

template<typename Pointer, Pointer p>
struct member_overload_traits;

template<typename Return, typename Class,  typename ... Args,
         Return(Class::* Pointer)(Args...)>
struct member_overload_traits<Return(Class::* const)(Args...), Pointer>
        : overload_traits<Return(Class&, Args...)>
{
    typedef Return(Class::*function_type)(Args...);
    constexpr static std::size_t cnt = sizeof...(Args);
    constexpr static std::make_index_sequence<cnt> seq{};

    template<std::size_t Idx>
    using type = std::tuple_element_t<Idx, std::tuple<Args...>>;

    template<typename ... Opts>
    static int try_invoke_no_string_impl(Class *cl,
                                         std::enable_if_t<!std::is_void_v<decltype((cl->*Pointer)(*std::declval<Opts>()...))>,  Tcl_Interp> * interp,
                                         Opts && ... opts)
    {
        const bool invocable = (!!opts && ...);
        if (invocable)
        {
            auto res = (cl->*Pointer)(*std::move(opts)...);
            auto obj = make_object(interp, std::move(res));
            Tcl_SetObjResult(interp, obj.get());
            return TCL_OK;
        }
        else
            return TCL_CONTINUE;
    }

    template<typename ... Opts>
    static int try_invoke_no_string_impl(Class *cl,
                                         std::enable_if_t<std::is_void_v<decltype((cl->*Pointer)(*std::declval<Opts>()...))>, Tcl_Interp>  * interp,
                                         Opts && ... opts)
    {
        const bool invocable = (!!opts && ...);
        if (invocable)
        {
            (cl->*Pointer)(*std::move(opts)...);
            return TCL_OK;
        }
        else
            return TCL_CONTINUE;
    }

    template<std::size_t ... Idx>
    static int try_invoke_no_string(Class *cl,
                                    Tcl_Interp * interp, int objc, Tcl_Obj * const objv[],
                                    std::index_sequence<Idx...> seq = {})
    {
        return try_invoke_no_string_impl(
                cl, interp,
                // if you get an error you're missing some tag_invokes.
                try_cast_without_implicit_string<type<Idx>>(interp, objv[Idx + 1])...);
    }


    template<typename ... Opts>
    static int invoke_impl(Class *cl,
                           std::enable_if_t<!std::is_void_v<decltype((cl->*Pointer)(*std::declval<Opts>()...))>, Tcl_Interp>  * interp,
                           Opts && ... opts)
    {
        const bool invocable = (!!opts && ...);
        if (invocable)
        {
            auto res = (cl->*Pointer)(*std::move(opts)...);
            auto obj = make_object(interp, std::move(res));
            Tcl_SetObjResult(interp, obj.get());
            return TCL_OK;
        }
        else
            return TCL_CONTINUE;
    }

    template<typename ... Opts>
    static int invoke_impl(Class *cl,
                           std::enable_if_t<std::is_void_v<decltype((cl->*Pointer)(*std::declval<Opts>()...))>, Tcl_Interp>  * interp,
                           Opts && ... opts)
    {
        const bool invocable = (!!opts && ...);
        if (invocable)
        {
            (cl->*Pointer)(*std::move(opts)...);
            return TCL_OK;
        }
        else
            return TCL_CONTINUE;
    }


    template<std::size_t ... Idx>
    static int invoke(Class *cl,
                      Tcl_Interp * interp, int objc, Tcl_Obj * const objv[],
                      std::index_sequence<Idx...> seq = {})
    {
        return invoke_impl(cl, interp,
                try_cast<type<Idx>>(interp, objv[Idx + 1])...);
    }


    template<std::size_t ... Idx>
    static int call_equal_impl(Class *cl,
                               Tcl_Interp * interp, int objc, Tcl_Obj * const objv[],
                               std::index_sequence<Idx...> seq = {})
    {
        const bool all_equal = (is_equal_type<type<Idx>>(objv[Idx + 1]->typePtr) && ... );
        if (all_equal)
            return invoke(cl, interp, objc, objv, seq);
        else
            return TCL_CONTINUE;
    }

    static int call_equal(Class *cl,Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
    {
        return call_equal_impl(cl, interp, objc-1, objv+1, seq);
    }

    static int call_equivalent(Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
    {
        return overload_traits<Return(Class&, Args...)>::call_equivalent(
                +[](Class & cl, Args ... args)
                {
                    return (cl.*Pointer)(static_cast<Args>(args)...);
                }, interp, objc-1, objv+1);
    }

    static int call_castable(Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
    {
        return overload_traits<Return(Class&, Args...)>::call_castable(
                +[](Class & cl, Args ... args)
                {
                    return (cl.*Pointer)(static_cast<Args>(args)...);
                }, interp, objc-1, objv+1);
    }

    static int call_with_string(Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
    {
        return overload_traits<Return(Class&, Args...)>::call_with_string(
                +[](Class & cl, Args ... args)
                {
                    return (cl.*Pointer)(static_cast<Args>(args)...);
                }, interp, objc-1, objv+1);
    }
};


template<typename T>
int class_command_impl (ClientData cdata, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
try
{
    auto p = static_cast<T*>(cdata);
    int res = TCL_CONTINUE;

    if (objc < 2)
    {
        constexpr char msg[] = "too few arguments";
        object_ptr obj = Tcl_NewStringObj(msg, sizeof(msg) - 1);
        Tcl_SetObjResult(interp, obj.get());
        return TCL_ERROR;
    }

    auto cmd = string_view(objv[1]->bytes, objv[1]->length);

    if (objc == 3 && cmd == ".get")
    {
        auto sv = string_view(objv[2]->bytes, objv[2]->length);
        enumerate_members<T>([&](auto d){
            if (sv == d.name)
            {
                auto ob = make_object(interp, p->*d.pointer);
                Tcl_SetObjResult(interp, ob.get());
                res = TCL_OK;
            }
        });

        if (res != TCL_CONTINUE)
            return res;
    }

    if (objc == 4 && cmd == ".set")
    {
        auto sv = string_view(objv[2]->bytes, objv[2]->length);
        enumerate_members<T>([&](auto d){
            if (sv == d.name)
            {
                using type = std::decay_t<decltype(p->*d.pointer)>;
                p->*d.pointer = cast<type>(interp, objv[3]);
                Tcl_SetObjResult(interp, objv[3]);
                res = TCL_OK;
            }
        });

        if (res != TCL_CONTINUE)
            return res;
    }

    // call_equal
    enumerate_members<T, static_cast<describe::modifiers>(describe::mod_public | describe::mod_function)>(
            [&](auto d)
            {
                using traits = member_overload_traits<decltype(d.pointer), d.pointer>;
                if (d.name == cmd && res == TCL_CONTINUE)
                    res = traits::call_equal(p, interp, objc, objv);
            });
    if (res != TCL_CONTINUE)
        return res;

    // call_equivalent
    enumerate_members<T, static_cast<describe::modifiers>(describe::mod_public | describe::mod_function)>(
            [&](auto d)
            {
                using traits = member_overload_traits<decltype(d.pointer), d.pointer>;
                if (d.name == cmd && res == TCL_CONTINUE)
                    res = traits::call_equivalent(interp, objc, objv);
            });
    if (res != TCL_CONTINUE)
        return res;

    // call_castable
    enumerate_members<T, static_cast<describe::modifiers>(describe::mod_public | describe::mod_function)>(
            [&](auto d)
            {
                using traits = member_overload_traits<decltype(d.pointer), d.pointer>;
                if (d.name == cmd && res == TCL_CONTINUE)
                    res = traits::call_castable(interp, objc, objv);
            });

    if (res != TCL_CONTINUE)
        return res;

    // call_with_string
    enumerate_members<T, static_cast<describe::modifiers>(describe::mod_public | describe::mod_function)>(
            [&](auto d)
            {
                using traits = member_overload_traits<decltype(d.pointer), d.pointer>;
                if (d.name == cmd && res == TCL_CONTINUE)
                    res = traits::call_with_string(interp, objc, objv);
            });

    if (res != TCL_CONTINUE)
        return res;

  err:
    constexpr char msg[] = "no matching overload";
    object_ptr obj = Tcl_NewStringObj(msg, sizeof(msg) - 1);
    Tcl_SetObjResult(interp, obj.get());
    return TCL_ERROR;
}
catch (...) {
    auto obj = ::boost::tcl::make_exception_object();
    Tcl_SetObjResult(interp, obj.get());
    return TCL_ERROR;
}

template<typename T>
class_commands * make_class_command(T* ptr, Tcl_Interp * interp, const char * name)
{
    printf("Creating command '%s'\n", name);
    std::string nm = "*";
    nm += name;
    auto res = new class_commands{
          interp,
          Tcl_CreateObjCommand(interp, nm.c_str()/*, name*/, &class_command_impl<T>, ptr,
                               [](void * p)
                               {
                                    printf("Delete ct %p\n", p);
                               }),
          +[](void *ptr, Tcl_Interp * interp, const char * name)
          {
                return make_class_command(static_cast<T*>(ptr), interp, name);
          }};
    Tcl_CmdInfo ci;
    Tcl_GetCommandInfo(interp, name, &ci);

    return res;
}

}

}

#endif //BOOST_TCL_COMMAND_HPP
