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
#include <boost/tcl/detail/overload_traits.hpp>


namespace boost::tcl
{

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
        add_default_ctor<Class>(std::is_default_constructible<Class>{});
        add_copy_ctor<Class>(std::is_copy_constructible<Class>{});
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

        struct impl_deleter
        {
          void(*deleter_func)(void*) = nullptr;
          void operator()(void * ptr)
          {
            if (deleter_func)
                deleter_func(ptr);
          }
        };

        std::unique_ptr<void, impl_deleter> impl_;

        int call_equal(Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
        {
            return call_equal_(impl_.get(), interp, objc, objv);
        }
        int call_equivalent(Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
        {
            return call_equivalent_(impl_.get(), interp, objc, objv);
        }
        int call_castable(Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
        {
            return call_castable_(impl_.get(), interp, objc, objv);
        }
        int call_with_string(Tcl_Interp * interp, int objc, Tcl_Obj * const objv[])
        {
            return call_string_(impl_.get(), interp, objc, objv);
        }
    };
    ~sub_command()
    {

    }
  private:

    template<typename Class>
    void add_default_ctor(std::false_type) {}
    template<typename Class>
    void add_default_ctor(std::true_type)  { add_constructor<Class()>(); }

    template<typename Class>
    void add_copy_ctor(std::false_type) {}
    template<typename Class>
    void add_copy_ctor(std::true_type)  { add_constructor<Class(const Class &)>(); }

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
                       std::unique_ptr<void, overload_t::impl_deleter>{reinterpret_cast<void*>(f), overload_t::impl_deleter{}}};
        overloads_.push_back(std::move(ovl));
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
                       std::unique_ptr<void, overload_t::impl_deleter>{reinterpret_cast<void*>(f), overload_t::impl_deleter{}}};
        overloads_.push_back(std::move(ovl));
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

                for (auto & can: candidates) // equal match
                    if (auto res = can.call_equal(interp, objc, objv); res != TCL_CONTINUE)
                        return res;

                for (auto & can: candidates) // equivalent match
                    if (auto res = can.call_equivalent(interp, objc, objv); res != TCL_CONTINUE)
                        return res;

                for (auto & can: candidates) // castable match
                    if (auto res = can.call_castable(interp, objc, objv); res != TCL_CONTINUE)
                        return res;

                for (auto & can: candidates) // string match
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

inline command & create_command(const interpreter_ptr & interp, const char * name)
{
  return create_command(interp.get(), name);
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
                    res = traits::call_equal(interp, objc, objv);
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
    std::string nm = "*";
    nm += name;
    auto res = new class_commands{
          interp,
          Tcl_CreateObjCommand(interp, nm.c_str()/*, name*/, &class_command_impl<T>, ptr,
                               [](void * p)
                               {
                                    delete static_cast<T*>(p);
                               }),
          +[](void *ptr, Tcl_Interp * interp, const char * name)
          {
              return make_class_command(static_cast<T*>(ptr), interp, name);
          }};
    return res;
}

}

}

#endif //BOOST_TCL_COMMAND_HPP
