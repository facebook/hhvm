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

#include <runtime/ext/asio/asio_session.h>
#include <runtime/ext/ext_closure.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_THREAD_LOCAL_PROXY(AsioContext, false, AsioSession::s_ctx);
IMPLEMENT_THREAD_LOCAL_PROXY(ObjectData, false, AsioSession::s_on_failed_cb);


void AsioSession::Init() {
  s_ctx.set(nullptr);
  s_on_failed_cb.set(nullptr);
}

void AsioSession::SetOnFailedCallback(CObjRef on_failed_cb) {
  if (!on_failed_cb.isNull()) {
    on_failed_cb->incRefCount();
  }

  if (s_on_failed_cb.get()) {
    decRefObj(s_on_failed_cb.get());
  }

  s_on_failed_cb.set(on_failed_cb.get());
}

void AsioSession::OnFailed(CObjRef exception) {
  ObjectData* cb = s_on_failed_cb.get();
  if (cb) {
    f_call_user_func_array(cb, Array::Create(exception));
  }
}

///////////////////////////////////////////////////////////////////////////////
}
