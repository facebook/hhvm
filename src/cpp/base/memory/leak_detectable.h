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

#ifndef __HPHP_LEAK_DETECTABLE_H__
#define __HPHP_LEAK_DETECTABLE_H__

#include <util/stack_trace.h>
#include <util/lock.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * Detect memory leaks by manual instrumentation of a specific class.
 */
class LeakDetectable {
public:
  /**
   * LeakDetectable object based detection:
   *
   *   1. Derive a class from LeakDetectable and implement dump().
   *   2. Optionally call markReachable() to exclude some reporting.
   *   3. AdminRequestHandler has leak-on/off commands to capture leaks.
   */
  static int TotalCount;
  static int TotalAllocated;
  static void BeginLeakChecking();
  static int EndLeakChecking(std::string &dumps, int sampling);

  /**
   * Malloc based detection:
   *
   *   1. Only sample some mallocs, so not to slow down server.
   *   2. Call BeginMallocSampling() after initialization to avoid reporting
   *      of long-lived objects.
   *   4. Call EndMallocSampling() to report leaks. Use "cutoff" to filter out
   *      newly created objects that haven't got any chance to get deleted yet.
   */
  static int MallocSampling;
  static int StackTraceGroupLevel;
  static void BeginMallocSampling();
  static void EndMallocSampling(std::string &dumps, int cutoff);

  /**
   * Log malloc peak usage for web pages.
   */
  static void EnableMallocStats(bool enable);
  static void LogMallocStats();

public:
  LeakDetectable();
  virtual ~LeakDetectable();

  /**
   * Mark an object to be supposedly reachable, so to exclude from reports.
   */
  void markReachable();

  /**
   * Debug dump of a leaked item.
   */
  virtual void dump(std::string &out) = 0;

private:
  typedef std::map<LeakDetectable*, StackTrace> StackTraceMap;

  static Mutex LeakMutex;
  static StackTraceMap AllocStackTraces;
  static bool SuppressStackTrace(const std::string &st);

  bool m_reachable;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif /* __HPHP_LEAK_DETECTABLE_H__ */
