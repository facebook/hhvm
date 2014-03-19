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
#ifndef incl_HPHP_DEBUGGABLE_H_
#define incl_HPHP_DEBUGGABLE_H_

#include "hphp/runtime/base/type-string.h"

#include <string>
#include <utility>
#include <vector>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * Implement this interface to report information to debugger or execute
 * debugger commands.
 */
class IDebuggable {
public:
  enum Support {
    SupportInfo = 1,
    SupportDump = 2,
    SupportVerb = 4,
  };

  typedef std::pair<const char*, std::string> InfoEntry;
  typedef std::vector<InfoEntry> InfoVec;

public:
  static void Add(InfoVec& out, const char* name, const std::string& value);
  static void AddServerStats(InfoVec& out, const char* name,
                             const char* statsName = nullptr);

  static std::string FormatNumber(const char* fmt, int64_t n);
  static std::string FormatSize(int64_t size);
  static std::string FormatTime(int64_t milliSeconds);

public:
  virtual ~IDebuggable() {}

  /**
   * Returns a map of those support bits. Tells caller which function can be
   * called.
   */
  virtual int debuggerSupport() {
    return 0;
  }

  /**
   * Fill up vector with summary information.
   */
  virtual void debuggerInfo(InfoVec& info) {
  }

  /**
   * Dump detailed information to return string.
   */
  virtual String debuggerDump() {
    return String();
  }

  /**
   * Execute a debugger action.
   */
  virtual String debuggerVerb(const std::string& verb,
                              const std::vector<std::string>& args) {
    return String();
  }
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_DEBUGGABLE_H_
