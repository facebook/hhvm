/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#ifndef incl_EXT_ASIO_SERVER_TASK_WAIT_HANDLE_H_
#define incl_EXT_ASIO_SERVER_TASK_WAIT_HANDLE_H_

#include "hphp/runtime/ext/asio/asio-external-thread-event.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * A ServerTaskEvent is an external thread ASIO event where the external thread
 * is an HPHP server job.
 *
 * The templated server class is expected to implement a TaskResult method.
 * At present, only XboxServer uses this event, so we adopt its particular
 * interface.
 */
template<class TServer, class TTransport>
struct ServerTaskEvent final : AsioExternalThreadEvent {
  ServerTaskEvent() {}
  ServerTaskEvent(const ServerTaskEvent&) = delete;
  ServerTaskEvent& operator=(const ServerTaskEvent&) = delete;

  ~ServerTaskEvent() override {
    if (m_job) m_job->decRefCount();
  }

  void finish() {
    markAsFinished();
  }

  void setJob(TTransport *job) {
    job->incRefCount();
    m_job = job;
  }

 protected:
  void unserialize(TypedValue& result) final {
    if (UNLIKELY(!m_job)) {
      SystemLib::throwInvalidOperationExceptionObject(
        "The async operation was incorrectly initialized.");
    }

    Variant ret;

    int code = TServer::TaskResult(m_job, 0, &ret);
    if (code != 200) {
      SystemLib::throwExceptionObject(ret);
    }

    tvDup(*ret.asTypedValue(), result);
  }

 private:

  TTransport *m_job{nullptr};
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_EXT_ASIO_SERVER_TASK_WAIT_HANDLE_H_
