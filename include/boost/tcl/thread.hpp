//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_TCL_THREAD_HPP
#define BOOST_TCL_THREAD_HPP

#include <tcl.h>
#include <boost/throw_exception.hpp>
#include <boost/tcl/exception.hpp>

namespace boost::tcl
{


struct thread
{
  using id = Tcl_ThreadId;
  thread(
      void (*raw_function)(),
      int stack_size = TCL_THREAD_STACK_DEFAULT,
      int flags = TCL_THREAD_JOINABLE)
      : joinable_((flags & TCL_THREAD_NOFLAGS) != 0u)
  {
    if (Tcl_CreateThread(
        &id_, +[](void * raw) {using type_t = void (*)(); reinterpret_cast<type_t>(raw)();},
        reinterpret_cast<void*>(raw_function), stack_size, flags))
      boost::throw_exception(std::runtime_error("Couldn't create thread"));

  }

  template<typename Function>
  thread(Function && func,
         int stack_size = TCL_THREAD_STACK_DEFAULT,
         int flags = TCL_THREAD_JOINABLE)
         : joinable_((flags & TCL_THREAD_NOFLAGS) != 0u)
  {
    using func_t = std::decay_t<Function>;
    if (Tcl_CreateThread(
        &id_,
        +[](void * raw)
        {
          std::unique_ptr<func_t> ptr{reinterpret_cast<func_t*>(raw)};
          (*ptr)();
        },
        new func_t(std::forward<Function>(func)), stack_size, flags))
      boost::throw_exception(std::runtime_error("Couldn't create thread"));
  }

  explicit thread(id id) : id_(id), joinable_(false) {}

  bool joinable() const
  {
    return joinable_;
  }

  int join()
  {
    int res = 0u;
    if (Tcl_JoinThread(id_, &res))
      boost::throw_exception(std::runtime_error("Couldn't join thread"));

    return res;
  }
  id get_id() {return id_;}

  void alert()
  {
    Tcl_ThreadAlert(id_);
  }

  void queue_event(Tcl_Event * event, Tcl_QueuePosition position = TCL_QUEUE_TAIL)
  {
    Tcl_ThreadQueueEvent(id_, event, position);
  }

 private:
  id id_;
  bool joinable_;
};

namespace this_thread
{

inline thread::id id()
{
  return Tcl_GetCurrentThread();
}
inline void queue_event(Tcl_Event * event, Tcl_QueuePosition position = TCL_QUEUE_TAIL)
{
  Tcl_QueueEvent(event, position);
}

}


}

#endif //BOOST_TCL_THREAD_HPP
