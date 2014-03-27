/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_UTIL_HARDWARE_COUNTER_H_
#define incl_HPHP_UTIL_HARDWARE_COUNTER_H_

#include "hphp/util/thread-local.h"

#include <cstdint>
#include <vector>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class Array;
class String;

#ifndef NO_HARDWARE_COUNTERS

class InstructionCounter;
class LoadCounter;
class StoreCounter;

struct PerfTable {
  const char* name;
  uint32_t type;
  uint64_t config;
};

class HardwareCounterImpl;

class HardwareCounter {
public:
  HardwareCounter();
  ~HardwareCounter();

  static void Reset();
  static int64_t GetInstructionCount();
  static int64_t GetLoadCount();
  static int64_t GetStoreCount();
  static bool SetPerfEvents(const String& events);
  static void GetPerfEvents(Array& ret);
  static void ClearPerfEvents();

  void reset();
  int64_t getInstructionCount();
  int64_t getLoadCount();
  int64_t getStoreCount();
  bool eventExists(char* event);
  bool addPerfEvent(char* event);
  bool setPerfEvents(const String& events);
  void getPerfEvents(Array& ret);
  void clearPerfEvents();

  static DECLARE_THREAD_LOCAL_NO_CHECK(HardwareCounter, s_counter);
  bool m_countersSet;
private:
  InstructionCounter* m_instructionCounter;
  LoadCounter* m_loadCounter;
  StoreCounter* m_storeCounter;
  std::vector<HardwareCounterImpl*> m_counters;
};

#else // NO_HARDWARE_COUNTERS

/* Stub implementation for platforms without hardware counters (non-linux)
 * This mock class pretends to track performance events, but just returns
 * static values, so it doesn't even need to worry about thread safety
 * for the one static instance of itself.
 */
class HardwareCounter {
public:
  HardwareCounter() : m_countersSet(false) { }
  ~HardwareCounter() { }

  static void Reset() { s_counter.reset(); }
  static int64_t GetInstructionCount() {
    return s_counter.getInstructionCount();
  }
  static int64_t GetLoadCount() { return s_counter.getLoadCount(); }
  static int64_t GetStoreCount() { return s_counter.getStoreCount(); }
  static bool SetPerfEvents(const String& events) {
    return s_counter.setPerfEvents(events);
  }
  static void GetPerfEvents(Array& ret) { s_counter.getPerfEvents(ret); }
  static void ClearPerfEvents() { s_counter.clearPerfEvents(); }

  void reset() { }
  int64_t getInstructionCount() { return 0; }
  int64_t getLoadCount() { return 0; }
  int64_t getStoreCount() { return 0; }
  bool eventExists(char* event) { return false; }
  bool addPerfEvent(char* event) { return false; }
  bool setPerfEvents(const String& events) { return false; }
  void getPerfEvents(Array& ret) { }
  void clearPerfEvents() { }

  // Normally exposed by DECLARE_THREAD_LOCAL_NO_CHECK
  void getCheck() { }

  static HardwareCounter s_counter;
  bool m_countersSet;
};

#endif // NO_HARDWARE_COUNTERS

///////////////////////////////////////////////////////////////////////////////
}

#endif
