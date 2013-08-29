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
#ifndef incl_HPHP_SIMPLE_COUNTER_H_
#define incl_HPHP_SIMPLE_COUNTER_H_

#include "hphp/runtime/base/request-local.h"
#include "hphp/util/stack-trace.h"
#include <map>
#include <string>
#include <vector>

// #define ENABLE_SIMPLE_COUNTER 1

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class SimpleCounter : public RequestEventHandler {
public:
  SimpleCounter() { }

  virtual void requestInit();
  virtual void requestShutdown();

  static void Count(const std::string &name);

  static bool Enabled;
  static int SampleStackCount;
  static int SampleStackDepth;

private:
  typedef std::map<std::string, int> CounterMap;
  typedef std::map<std::string, std::vector<std::string> > StacktraceMap;

  class Comparer {
  public:
    explicit Comparer(CounterMap &cm) : m_map(cm) { }
    bool operator()(const std::string &s1, const std::string &s2);

    CounterMap &m_map;
  };

  CounterMap m_counters;
  StacktraceMap m_stacks;
};

#ifdef ENABLE_SIMPLE_COUNTER
#define COUNTING(n) SimpleCounter::Count(n)
#else
#define COUNTING(n)
#endif

///////////////////////////////////////////////////////////////////////////////
}

#endif
