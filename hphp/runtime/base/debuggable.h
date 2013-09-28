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
#ifndef incl_HPHP_DEBUGGABLE_H_
#define incl_HPHP_DEBUGGABLE_H_

#include "hphp/util/base.h"
#include "hphp/runtime/base/string-buffer.h"
#include "hphp/runtime/base/complex-types.h"

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

  typedef std::pair<const char *, std::string> InfoEntry;
  typedef std::vector<InfoEntry> InfoVec;

public:
  static void Add(InfoVec &out, const char *name, const std::string &value);
  static void AddServerStats(InfoVec &out, const char *name,
                             const char *statsName = nullptr);

  static std::string FormatNumber(const char *fmt, int64_t n);
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
  virtual void debuggerInfo(InfoVec &info) {
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
  virtual String debuggerVerb(const std::string &verb, const StringVec &args) {
    return String();
  }
};

#define HPHPD_SETTINGS \
  HPHPD_SETTING(BypassCheck,         bool,  false)         \
  HPHPD_SETTING(PrintLevel,          int,   -1)            \
  HPHPD_SETTING(StackArgs,           bool,  true)          \

class DebuggerSettings {
public:
#define HPHPD_SETTING(name, type, defval) type m_s##name;
  HPHPD_SETTINGS
#undef HPHPD_SETTING
  bool dummy;

  DebuggerSettings() :
#define HPHPD_SETTING(name, type, defval) m_s##name(defval),
  HPHPD_SETTINGS
#undef HPHPD_SETTING
  dummy(false) {}
};

#define DECLARE_DBG_SETTING                        \
  DebuggerSettings m_dbgSettings;                  \

#define HPHPD_SETTING(name, type, defval)          \
type getDebugger##name () const {                  \
  return m_dbgSettings.m_s##name;                  \
}                                                  \
void setDebugger##name (type in##name) {           \
  m_dbgSettings.m_s##name = in##name;              \
}                                                  \


#define DECLARE_DBG_SETTING_ACCESSORS              \
HPHPD_SETTINGS

// leaving HPHPD_SETTING defined so that DECLARE_DBG_SETTING_ACCESSORS is
// effective

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_DEBUGGABLE_H_
