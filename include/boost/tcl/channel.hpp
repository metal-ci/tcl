//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_TCL_CHANNEL_HPP
#define BOOST_TCL_CHANNEL_HPP

#include <tcl.h>
#include <boost/tcl/class.hpp>

namespace boost::tcl
{

namespace detail
{

template<typename Impl>
Tcl_DriverGetHandleProc * getHandleProcImpl(rank<0>) { return nullptr;}

template<typename Impl, typename = decltype(&Impl::get_handle)>
Tcl_DriverGetHandleProc  * getHandleProcImpl(rank<1>)
{
  return +[](ClientData instanceData, int direction, ClientData *handlePtr)
          {
            *handlePtr = static_cast<Impl*>(instanceData)->get_handle(direction);
          };
}

template<typename Impl>
Tcl_DriverFlushProc * getFlushProc(rank<0>) { return nullptr;}

template<typename Impl, typename = decltype(&Impl::flush)>
Tcl_DriverFlushProc * getFlushProc(rank<1>)
{
  return +[](ClientData instanceData)
  {
    static_cast<Impl*>(instanceData)->flush();
  };
}

template<typename Impl>
Tcl_DriverThreadActionProc * getThreadActionProc(rank<0>) { return nullptr;}

template<typename Impl, typename = decltype(&Impl::thread_action)>
Tcl_DriverThreadActionProc * getThreadActionProc(rank<1>)
{
  return +[](ClientData instanceData, int action)
  {
    static_cast<Impl*>(instanceData)->thread_action(action);
  };
}

template<typename Impl>
Tcl_DriverTruncateProc * getTruncateProc(rank<0>) { return nullptr;}

template<typename Impl, typename = decltype(&Impl::truncate)>
Tcl_DriverTruncateProc * getTruncateProc(rank<1>)
{
  return +[](ClientData instanceData, Tcl_WideInt length)
  {
    static_cast<Impl*>(instanceData)->truncate(length);
  };
}
template<typename Impl>
const Tcl_ChannelType channel_type{
  /*.typeName=*/  tag_invoke(detail::get_class_name_tag<Impl>{}).data(),
  /*.version=*/   TCL_CHANNEL_VERSION_5,
  /*.closeProc=*/ TCL_CLOSE2PROC,
  /*.inputProc=*/ +[](ClientData instanceData, char *buf, int toRead, int *errorCodePtr)
      {
        return static_cast<Impl*>(instanceData)->input(buf, toRead, *errorCodePtr);
      },
  /*.outputProc=*/+[](ClientData instanceData, const char *buf, int toWrite, int *errorCodePtr)
      {
        return static_cast<Impl*>(instanceData)->output(buf, toWrite, *errorCodePtr);
      },
  /*.seekProc=*/+[](ClientData instanceData, long offset, int mode, int *errorCodePtr)
      {
        return static_cast<Impl*>(instanceData)->seek(offset, mode, *errorCodePtr);
      },
  /*.setOptionProc=*/+[](ClientData instanceData, Tcl_Interp *interp, const char *optionName, const char *value)
      {
        return static_cast<Impl*>(instanceData)->set_option(interp, optionName, value);
      },
  /*.getOptionProc=*/+[](ClientData instanceData, Tcl_Interp *interp, const char *optionName, Tcl_DString *dsPtr)
      {
        return static_cast<Impl*>(instanceData)->get_option(interp, optionName, *dsPtr);
      },
  /*.watchProc=*/+[](ClientData instanceData, int mask)
      {
        static_cast<Impl*>(instanceData)->watch(mask);
      },
  /*.getHandleProc=*/detail::getHandleProcImpl<Impl>(detail::rank<1>{}),
  /*.close2Proc=*/+[](ClientData instanceData, Tcl_Interp *interp, int flags)
              {
                return static_cast<Impl*>(instanceData)->close(interp, flags);
              },
  /*.blockModeProc=*/+[](ClientData instanceData, int mode)
              {
                return static_cast<Impl*>(instanceData)->block_mode(mode == TCL_MODE_BLOCKING);
              },
  /*.flushProc=*/detail::getFlushProc<Impl>(detail::rank<1>{}),
  /*.handlerProc=*/+[](ClientData instanceData, int interestMask)
              {
                return static_cast<Impl*>(instanceData)->handler(interestMask);
              },
  /*.wideSeekProc=*/+[](ClientData instanceData, Tcl_WideInt offset, int mode, int *errorCodePtr) -> Tcl_WideInt
  {
    return static_cast<Impl*>(instanceData)->seek(offset, mode, *errorCodePtr);
  },
  /*.threadActionProc=*/detail::getThreadActionProc<Impl>(detail::rank<1>{}),
  /*.truncateProc=*/detail::getTruncateProc<Impl>(detail::rank<1>{})
};

}

template<typename Impl>
Tcl_Channel create_channel(Impl & impl, const char * name, int flags = TCL_WRITABLE | TCL_READABLE)
{
  return Tcl_CreateChannel(&detail::channel_type<Impl>, name, &impl, flags);
}


}


#endif //BOOST_TCL_CHANNEL_HPP
