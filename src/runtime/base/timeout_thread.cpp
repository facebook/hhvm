/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <sys/mman.h>

#include <runtime/base/timeout_thread.h>
#include <runtime/base/runtime_option.h>
#include <util/lock.h>
#include <util/logger.h>

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

TimeoutThread::TimeoutThread(int timeoutSeconds)
  : m_nextId(0), m_stopped(false), m_timeoutSeconds(timeoutSeconds) {
  m_eventBase = event_base_new();

  // We need to open the pipe here because worker threads can start
  // before the timeout thread starts
  m_pipe.open();
}

TimeoutThread::~TimeoutThread() {
  event_base_free(m_eventBase);
}

void TimeoutThread::registerRequestThread(RequestInjectionData* data) {
  ASSERT(data);
  data->timeoutSeconds = m_timeoutSeconds;

  {
    Lock l(this);
    int id = m_nextId++;
    ASSERT(!mapContains(m_clients, id));
    m_clients[id].data = data;
    m_pendingIds.push(id);
  }
  notifyPipe();
}

void TimeoutThread::removeRequestThread(RequestInjectionData* data) {
  ASSERT(data);
  {
    Lock l(this);
    for (auto& pair : m_clients) {
      if (pair.second.data == data) {
        m_pendingIds.push(pair.first);
        pair.second.data = nullptr;
        break;
      }
    }
  }
  notifyPipe();
}

void TimeoutThread::checkForNewWorkers() {
  // If m_timeoutSeconds is not a positive number, then workers threads
  // are allowed to run forever, so don't bother creating timers for the
  // workers
  if (m_timeoutSeconds <= 0) {
    return;
  }

  Lock lock(this);
  for (; !m_pendingIds.empty(); m_pendingIds.pop()) {
    int id = m_pendingIds.front();
    ASSERT(mapContains(m_clients, id));
    ClientThread& ct = m_clients[id];

    if (ct.data != nullptr) {
      // This is a new thread and we have to register its timeout event

      struct timeval timeout;
      timeout.tv_usec = 0;
      // +2 to make sure when it times out, this equation always holds:
      //   time(0) - RequestInjection::s_reqInjectionData->started >=
      //     m_timeoutSeconds
      timeout.tv_sec = m_timeoutSeconds + 2;

      event_set(&ct.e, id, 0, on_timer, this);
      event_base_set(m_eventBase, &ct.e);
      event_add(&ct.e, &timeout);
    } else {
      // This was a deleted thread and we have to remove its timeout event
      event_del(&ct.e);
      m_clients.erase(id);
    }
  }
}

void TimeoutThread::drainPipe() {
  struct pollfd fdArray[1];
  fdArray[0].fd = m_pipe.getOut();
  fdArray[0].events = POLLIN;
  while (poll(fdArray, 1, 0) > 0) {
    char buf[256];
    read(m_pipe.getOut(), buf, 256);
  }
}

void TimeoutThread::notifyPipe() {
  if (write(m_pipe.getIn(), "", 1) < 0) {
    Logger::Warning("Error notifying the timeout thread that an event has "
                    "happened");
  }
}

void TimeoutThread::run() {
  event_set(&m_eventPipe, m_pipe.getOut(), EV_READ|EV_PERSIST,
            on_thread_stop, m_eventBase);
  event_base_set(m_eventBase, &m_eventPipe);
  event_add(&m_eventPipe, NULL);

  while (!m_stopped) {
    checkForNewWorkers();
    event_base_loop(m_eventBase, EVLOOP_ONCE);
    drainPipe();
  }

  for (auto& pair : m_clients) {
    event_del(&pair.second.e);
  }
  event_del(&m_eventPipe);
}

void TimeoutThread::stop() {
  m_stopped = true;
  notifyPipe();
}

void TimeoutThread::onTimer(int index) {
  Lock l(this);
  ASSERT(mapContains(m_clients, index));
  ClientThread& ct = m_clients[index];
  if (ct.data == nullptr) {
    // The thread has been deleted but we haven't processed it
    // yet. This is ok: just do nothing.
    return;
  }

  event *e = &ct.e;
  event_del(e);

  RequestInjectionData *data = ct.data;
  ASSERT(data);
  struct timeval timeout;
  timeout.tv_usec = 0;
  if (data->started > 0) {
    time_t now = time(0);
    int delta = now - data->started;
    if (delta >= m_timeoutSeconds) {
      timeout.tv_sec = m_timeoutSeconds + 2;
      if (hhvm) {
        Lock l(data->surpriseLock);
        data->setTimedOutFlag();
        if (data->surprisePage) {
          mprotect(data->surprisePage, sizeof(void*), PROT_NONE);
        }
      } else {
        data->setTimedOutFlag();
      }
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
