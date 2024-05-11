//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef METAL_TCL_EVENT_HPP
#define METAL_TCL_EVENT_HPP

#include <tcl.h>
#include <cstdint>
#include <utility>

namespace metal::tcl
{

struct event_source
{
  virtual void setup(int flags) = 0;
  virtual void check(int flags) = 0;

  event_source()
  {
    Tcl_CreateEventSource(
        &setup_impl_,
        &check_impl_,
        this);
  }

  event_source(const event_source &) = delete;

  ~event_source()
  {
    Tcl_DeleteEventSource(&setup_impl_,
                          &check_impl_,
                          this);
  }
 private:
  static void setup_impl_(ClientData clientData, int flags)
  {
    static_cast<event_source*>(clientData)->setup(flags);
  }
  static void check_impl_(ClientData clientData, int flags)
  {
    static_cast<event_source*>(clientData)->check(flags);
  }
};

template<typename Predicate>
void delete_events(Predicate & predicate)
{
  Tcl_DeleteEvents(
      [](Tcl_Event *evPtr, ClientData clientData)
      {
        return (*static_cast<Predicate*>(clientData))(evPtr) ? 0 : 1;
      }, &predicate);
}


template<typename Impl,
         typename = std::enable_if_t<std::is_trivially_destructible_v<Impl>>>
struct event : Tcl_Event
{
  template<typename Impl_>
  event(Impl_ && impl) : Tcl_Event{
        +[](Tcl_Event *evPtr, int flags) -> int
        {
            return static_cast<event*>(evPtr)->impl_(flags) ? 1 : 0;
        },
        nullptr}, impl_(std::forward<Impl_>(impl)) {}

  event(const event &) = delete;
  void* operator new  ( std::size_t count )
  {
    auto alloc = Tcl_Alloc(count);
    return alloc;
  }
  void operator delete(void* ptr)
  {
    Tcl_Free(static_cast<char*>(ptr));
  }
 private:
  Impl impl_;
};

template<typename Impl>
event(Impl && ) -> event<std::decay_t<Impl>>;

}

#endif //METAL_TCL_EVENT_HPP
