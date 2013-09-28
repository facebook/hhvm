/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/ext/asio/asio_external_thread_event.h"

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
class ServerTaskEvent : public AsioExternalThreadEvent {
 public:
  ServerTaskEvent() {}

  ~ServerTaskEvent() {
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
  void unserialize(Cell& result) const {
    if (UNLIKELY(!m_job)) {
      throw Object(SystemLib::AllocInvalidOperationExceptionObject(
        "The async operation was incorrectly initialized."));
    }

    Variant ret;

    int code = TServer::TaskResult(m_job, 0, ret);
    if (code != 200) {
      throw Object(SystemLib::AllocExceptionObject(ret));
    }

    cellDup(*ret.asCell(), result);
  }

 private:
  TTransport *m_job;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_EXT_ASIO_SERVER_TASK_WAIT_HANDLE_H_
