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

#include <runtime/base/timeout_thread.h>
#include <runtime/base/runtime_option.h>
#include <util/lock.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// statics

// class defined in runtime/base/types.h
static void on_timer(int fd, short events, void *context) {
  ((TimeoutThread*)context)->onTimer(fd);
}

static void on_thread_stop(int fd, short events, void *context) {
  event_base_loopbreak((struct event_base *)context);
}

///////////////////////////////////////////////////////////////////////////////

void TimeoutThread::DeferTimeout(int seconds) {
  RequestInjectionData &data = ThreadInfo::s_threadInfo->m_reqInjectionData;
  if (seconds > 0) {
    // cheating by resetting started to desired timestamp
    data.started = time(0) + (seconds - data.timeoutSeconds);
  } else {
    data.started = 0;
  }
}

///////////////////////////////////////////////////////////////////////////////

TimeoutThread::TimeoutThread(int timerCount, int timeoutSeconds)
  : m_index(0), m_stopped(false), m_timeoutSeconds(timeoutSeconds) {
  ASSERT(timerCount > 0);

  m_eventBase = event_base_new();
  m_eventTimeouts.resize(timerCount);
  m_timeoutData.resize(timerCount);
}

TimeoutThread::~TimeoutThread() {
  event_base_free(m_eventBase);
}

void TimeoutThread::registerRequestThread(RequestInjectionData* data) {
  ASSERT(data);
  data->timeoutSeconds = m_timeoutSeconds;

  Lock lock(this);
  ASSERT(m_index < (int)m_timeoutData.size());
  m_timeoutData[m_index++] = data;
  if (m_index == (int)m_timeoutData.size()) {
    notify();
  }
}

void TimeoutThread::run() {
  {
    Lock lock(this);
    while (m_index < (int)m_timeoutData.size()) {
      wait();
    }
    ASSERT(m_index == (int)m_timeoutData.size());
  }

  if (m_timeoutSeconds <= 0) {
    return;
  }

  struct timeval timeout;
  timeout.tv_usec = 0;

  // +2 to make sure when it times out, this equation always holds:
  //   time(0) - RequestInjection::s_reqInjectionData->started >=
  //     m_timeoutSeconds
  timeout.tv_sec = m_timeoutSeconds + 2;

  for (unsigned int i = 0; i < m_eventTimeouts.size(); i++) {
    event *e = &m_eventTimeouts[i];
    event_set(e, i, 0, on_timer, this);
    event_base_set(m_eventBase, e);
    event_add(e, &timeout);
  }

  m_pipeStop.open();
  event_set(&m_eventStop, m_pipeStop.getOut(), EV_READ|EV_PERSIST,
            on_thread_stop, m_eventBase);
  event_base_set(m_eventBase, &m_eventStop);
  event_add(&m_eventStop, NULL);

  while (!m_stopped) {
    event_base_loop(m_eventBase, EVLOOP_ONCE);
  }

  for (unsigned int i = 0; i < m_eventTimeouts.size(); i++) {
    event_del(&m_eventTimeouts[i]);
  }
  event_del(&m_eventStop);
}

void TimeoutThread::stop() {
  m_stopped = true;
  if (write(m_pipeStop.getIn(), "", 1) < 0) {
    // an error occured but we're in shutdown already, so ignore
  }
}

void TimeoutThread::onTimer(int index) {
  ASSERT(index >= 0 && index < (int)m_eventTimeouts.size());

  event *e = &m_eventTimeouts[index];
  event_del(e);

  RequestInjectionData *data = m_timeoutData[index];
  ASSERT(data);
  struct timeval timeout;
  timeout.tv_usec = 0;
  if (data->started > 0) {
    time_t now = time(0);
    int delta = now - data->started;
    if (delta >= m_timeoutSeconds) {
      timeout.tv_sec = m_timeoutSeconds + 2;
      data->timedout = true; // finally sure request is timed out
    } else {
      // Negative delta means start time was adjusted forward to give more time
      if (delta < 0) delta = 0;

      // otherwise, a new request started after we started the timer
      timeout.tv_sec = m_timeoutSeconds - delta + 2;
    }
  } else {
    // Another cycle of m_timeoutSeconds
    timeout.tv_sec = m_timeoutSeconds;
  }

  event_set(e, index, 0, on_timer, this);
  event_base_set(m_eventBase, e);
  event_add(e, &timeout);
}

///////////////////////////////////////////////////////////////////////////////
}
