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

#define _GNU_SOURCE 1
#include <util/hardware_counter.h>
#include <util/logger.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <asm/unistd.h>
#include <sys/prctl.h>
#include <linux/perf_event.h>

namespace HPHP { namespace Util {
///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_THREAD_LOCAL_NO_CHECK(HardwareCounter,
    HardwareCounter::s_counter);

class HardwareCounterImpl {
public:
  HardwareCounterImpl(int type, unsigned long config) : m_fd(-1) {
    memset (&pe, 0, sizeof (struct perf_event_attr));
    pe.type = type;
    pe.size = sizeof (struct perf_event_attr);
    pe.config = config;
    pe.disabled = 1;
    pe.pinned = 0;
    pe.exclude_kernel = 1;
    pe.exclude_hv = 1;
    pe.read_format =
      PERF_FORMAT_TOTAL_TIME_ENABLED|PERF_FORMAT_TOTAL_TIME_RUNNING;

#ifndef VALGRIND
    /*
     * perf_event_open(struct perf_event_attr *hw_event_uptr, pid_t pid,
     *                 int cpu, int group_fd, unsigned long flags)
     */
    m_fd = syscall(__NR_perf_event_open, &pe, 0, -1, -1, 0);
    if (m_fd < 0) {
      Logger::Verbose("perf_event_open failed with: %s",
          Util::safe_strerror(errno).c_str());
      return;
    }
    if (ioctl(m_fd, PERF_EVENT_IOC_ENABLE, 0) < 0) {
      Logger::Verbose("perf_event failed to enable: %s",
          Util::safe_strerror(errno).c_str());
      close();
      return;
    }
    reset();
#endif
  }

  ~HardwareCounterImpl() {
    close();
  }

  int64 read() {
    int64 count = 0;

#ifndef VALGRIND
    if (m_fd > 0) {
      int64 values[3];
      int ret;

      /*
       * read the count + scaling values
       *
       * It is not necessary to stop an event to read its value
       */
      ret = ::read(m_fd, values, sizeof(values));
      if (ret == sizeof(values)) {
        /*
         * scale count
         *
         * values[0] = raw count
         * values[1] = TIME_ENABLED
         * values[2] = TIME_RUNNING
         */
        if (values[2]) {
          count = (int64) ((double) values[0] * values[1] / values[2]);
        }
      }
    }
#endif
    return count;
  }

  void reset() {
    if (m_fd > 0 && ioctl (m_fd, PERF_EVENT_IOC_RESET, 0) < 0) {
      Logger::Verbose("perf_event failed to reset with: %s",
          Util::safe_strerror(errno).c_str());
    }
  }

private:
  int m_fd;
  struct perf_event_attr pe;

  void close() {
    if (m_fd > 0) {
      ::close(m_fd);
      m_fd = -1;
    }
  }
};

class InstructionCounter : public HardwareCounterImpl {
public:
  InstructionCounter() :
    HardwareCounterImpl(PERF_TYPE_HARDWARE, PERF_COUNT_HW_INSTRUCTIONS) {}
};

class LoadCounter : public HardwareCounterImpl {
public:
  LoadCounter() :
    HardwareCounterImpl(PERF_TYPE_HW_CACHE,
        (PERF_COUNT_HW_CACHE_L1D | ((PERF_COUNT_HW_CACHE_OP_READ) << 8))) {}
};

class StoreCounter : public HardwareCounterImpl {
public:
  StoreCounter() :
    HardwareCounterImpl(PERF_TYPE_HW_CACHE,
        PERF_COUNT_HW_CACHE_L1D | ((PERF_COUNT_HW_CACHE_OP_WRITE) << 8)) {}
};

HardwareCounter::HardwareCounter() {
  m_instructionCounter = new InstructionCounter();
  m_loadCounter = new LoadCounter();
  m_storeCounter = new StoreCounter();
}

HardwareCounter::~HardwareCounter() {
  delete m_instructionCounter;
  delete m_loadCounter;
  delete m_storeCounter;
}

void HardwareCounter::Reset(void) {
  s_counter->reset();
}

void HardwareCounter::reset(void) {
  m_instructionCounter->reset();
  m_storeCounter->reset();
  m_loadCounter->reset();
}

int64 HardwareCounter::GetInstructionCount() {
  return s_counter->getInstructionCount();
}

int64 HardwareCounter::getInstructionCount() {
  return m_instructionCounter->read();
}

int64 HardwareCounter::GetLoadCount() {
  return s_counter->getLoadCount();
}

int64 HardwareCounter::getLoadCount() {
  return m_loadCounter->read();
}

int64 HardwareCounter::GetStoreCount() {
  return s_counter->getStoreCount();
}

int64 HardwareCounter::getStoreCount() {
  return m_storeCounter->read();
}
///////////////////////////////////////////////////////////////////////////////
}}
