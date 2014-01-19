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

#ifndef incl_HPHP_SHM_COUNTER_H_
#define incl_HPHP_SHM_COUNTER_H_

#include <string.h>

// #define ENABLE_SHM_COUNTER

#define SHM_COUNTER_KEY 768

#define SHM_COUNTER_DEF(n) ShmCounter n;
#define SHM_COUNTER_INI(n) n(#n),

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class ShmCounter {
public:
  ShmCounter() {}
  explicit ShmCounter(const char *n) : count(0) {
    size_t size = sizeof(name);
    strncpy(name, n, size);
    name[size - 1] = '\0';
  }
  void dump();
  char name[64];
  long long count;
};
class ShmCounters {
public:
  SHM_COUNTER_DEF(dummy_def1)
  SHM_COUNTER_DEF(dummy_def2)
  // Add your real counter definition here


  SHM_COUNTER_DEF(dummy_defmax)

ShmCounters() :
  SHM_COUNTER_INI(dummy_def1)
  SHM_COUNTER_INI(dummy_def2)
  // Add your real counter initialization here

  dummy_defmax("dummy_defmax")
  {}

~ShmCounters();

  typedef void (*logError_t)(const char *fmt, ...);
  static bool initialize(bool create, logError_t logError = nullptr);
  static void dump();
public:
  static bool created;
  static int shmid;
  static logError_t logError;
  static ShmCounters *s_shmCounters;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_SHM_COUNTER_H_
