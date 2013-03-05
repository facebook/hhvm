/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
   | Copyright (c) 1997-2010 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include <runtime/ext/ext_asio.h>
#include <runtime/ext/ext_closure.h>
#include <runtime/ext/asio/asio_context.h>
#include <runtime/ext/asio/asio_session.h>
#include <system/lib/systemlib.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

void f_asio_enter_context() {
  // TODO: remove from API
}

void f_asio_exit_context() {
  // TODO: remove from API
}

Object f_asio_get_current() {
  return AsioSession::Get()->getCurrentWaitHandle();
}

void f_asio_set_on_failed_callback(CObjRef on_failed_cb) {
  if (!on_failed_cb.isNull() && !on_failed_cb.instanceof("closure")) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Unable to set asio on failed callback: on_failed_cb not a closure"));
    throw e;
  }

  AsioSession::Get()->setOnFailedCallback(on_failed_cb.get());
}

///////////////////////////////////////////////////////////////////////////////
}
