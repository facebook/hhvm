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

#ifndef __HPHP_UTIL_HARDWARE_COUNTER_H__
#define __HPHP_UTIL_HARDWARE_COUNTER_H__

#include <util/thread_local.h>
#include <linux/perf_event.h>
#include <runtime/base/complex_types.h>

namespace HPHP { namespace Util {
///////////////////////////////////////////////////////////////////////////////

class InstructionCounter;
class LoadCounter;
class StoreCounter;

struct PerfTable {
  const char *name;
  uint32_t type;
  uint64_t config;
};

class HardwareCounterImpl;

class HardwareCounter {
public:
  HardwareCounter();
  ~HardwareCounter();

  static void  Reset(void);
  static int64 GetInstructionCount(void);
  static int64 GetLoadCount(void);
  static int64 GetStoreCount(void);
  static bool SetPerfEvents(CStrRef events);
  static void GetPerfEvents(Array& ret);
  static void ClearPerfEvents();

  void  reset(void);
  int64 getInstructionCount(void);
  int64 getLoadCount(void);
  int64 getStoreCount(void);
  bool eventExists(char *event);
  bool addPerfEvent(char* event);
  bool setPerfEvents(CStrRef events);
  void getPerfEvents(Array& ret);
  void clearPerfEvents();

  static DECLARE_THREAD_LOCAL_NO_CHECK(HardwareCounter, s_counter);
  bool m_countersSet;
private:
  InstructionCounter *m_instructionCounter;
  LoadCounter *m_loadCounter;
  StoreCounter *m_storeCounter;
  std::vector<HardwareCounterImpl *> m_counters;
  bool m_pseudoEvents;
};
///////////////////////////////////////////////////////////////////////////////
}}

#endif // __HPHP_UTIL_HARDWARE_COUNTER_H__
