/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#ifndef __TIMEOUT_THREAD_H__
#define __TIMEOUT_THREAD_H__

#include <runtime/base/types.h>
#include <util/base.h>
#include <util/process.h>
#include <util/synchronizable.h>
#include <event.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class TimeoutThread : public Synchronizable {
public:
  static void DeferTimeout(int seconds);

public:
  TimeoutThread(int timerCount, int timeoutSeconds);
  ~TimeoutThread();

  void registerRequestThread(RequestInjectionData* data);
  void run();
  void stop();

  void onTimer(int index);

private:
  int m_index;
  bool m_stopped;

  event_base *m_eventBase;
  std::vector<event> m_eventTimeouts;
  std::vector<RequestInjectionData*> m_timeoutData;
  int m_timeoutSeconds;

  // signal to stop the thread
  event m_eventStop;
  CPipe m_pipeStop;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __TIMEOUT_THREAD_H__

