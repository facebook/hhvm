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
#include <runtime/ext/asio/asio_session.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_THREAD_LOCAL_PROXY(AsioSession, false, AsioSession::s_current);


void AsioSession::Init() {
  s_current.set(new AsioSession());
}

AsioSession::AsioSession() : m_contexts(), m_onFailedCallback(nullptr) {
}

uint16_t AsioSession::getCurrentWaitHandleDepth() {
  // have context and it's running
  if (!m_contexts.empty() && m_contexts.back()->isRunning()) {
    return m_contexts.back()->getCurrent()->getDepth();
  }

  // the current context is not running, look at the upper context
  // TODO: deprecate this once contexts are entered only by join()
  if (m_contexts.size() >= 2) {
    assert(m_contexts[m_contexts.size() - 2]->isRunning());
    return m_contexts[m_contexts.size() - 2]->getCurrent()->getDepth();
  }

  // we are the root
  return 0;
}

void AsioSession::setOnFailedCallback(ObjectData* on_failed_callback) {
  if (on_failed_callback) {
    on_failed_callback->incRefCount();
  }

  if (m_onFailedCallback) {
    decRefObj(m_onFailedCallback);
  }

  m_onFailedCallback = on_failed_callback;
}

void AsioSession::onFailed(CObjRef exception) {
  if (m_onFailedCallback) {
    f_call_user_func_array(m_onFailedCallback, Array::Create(exception));
  }
}

///////////////////////////////////////////////////////////////////////////////
}
