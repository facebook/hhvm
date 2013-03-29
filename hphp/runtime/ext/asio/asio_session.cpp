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
#include <system/lib/systemlib.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_THREAD_LOCAL_PROXY(AsioSession, false, AsioSession::s_current);

namespace {
  const context_idx_t MAX_CONTEXT_DEPTH = std::numeric_limits<context_idx_t>::max();
}

void AsioSession::Init() {
  s_current.set(new AsioSession());
}

AsioSession::AsioSession()
    : m_contexts(), m_onFailedCallback(nullptr), m_onStartedCallback(nullptr) {
}

void AsioSession::enterContext() {
  assert(!isInContext() || getCurrentContext()->isRunning());

  if (UNLIKELY(getCurrentContextIdx() >= MAX_CONTEXT_DEPTH)) {
    Object e(SystemLib::AllocInvalidOperationExceptionObject(
      "Unable to enter asio context: too many contexts open"));
    throw e;
  }

  m_contexts.push_back(new AsioContext());

  assert(static_cast<context_idx_t>(m_contexts.size()) == m_contexts.size());
  assert(isInContext());
  assert(!getCurrentContext()->isRunning());
}

void AsioSession::exitContext() {
  assert(isInContext());
  assert(!getCurrentContext()->isRunning());

  m_contexts.back()->exit(m_contexts.size());
  delete m_contexts.back();
  m_contexts.pop_back();

  assert(!isInContext() || getCurrentContext()->isRunning());
}

uint16_t AsioSession::getCurrentWaitHandleDepth() {
  assert(!isInContext() || getCurrentContext()->isRunning());
  return isInContext() ? getCurrentWaitHandle()->getDepth() : 0;
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
    try {
      vm_call_user_func(m_onFailedCallback, Array::Create(exception));
    } catch (Object callback_exception) {
      raise_warning("[asio] Ignoring exception thrown by onFailed callback");
    }
  }
}

void AsioSession::setOnStartedCallback(ObjectData* on_started_callback) {
  if (on_started_callback) {
    on_started_callback->incRefCount();
  }

  if (m_onStartedCallback) {
    decRefObj(m_onStartedCallback);
  }

  m_onStartedCallback = on_started_callback;
}

void AsioSession::onStarted(CObjRef wait_handle) {
  assert(m_onStartedCallback);
  try {
    vm_call_user_func(m_onStartedCallback, Array::Create(wait_handle));
  } catch (Object callback_exception) {
    raise_warning("[asio] Ignoring exception thrown by onStarted callback");
  }
}

///////////////////////////////////////////////////////////////////////////////
}
