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

#ifndef __HPHP_EVAL_DEBUGGER_CLIENT_SETTINGS_H__
#define __HPHP_EVAL_DEBUGGER_CLIENT_SETTINGS_H__

#define HPHPD_CLIENT_SETTINGS \
  HPHPD_CLIENT_SETTING(ApiModeSerialize,    bool,  false)         \
  HPHPD_CLIENT_SETTING(MaxCodeLines,        int,   -1)            \

class DebuggerClientSettings {
public:
#define HPHPD_CLIENT_SETTING(name, type, defval) type m_s##name;
  HPHPD_CLIENT_SETTINGS
#undef HPHPD_CLIENT_SETTING
  bool dummy;

  DebuggerClientSettings() :
#define HPHPD_CLIENT_SETTING(name, type, defval) m_s##name(defval),
  HPHPD_CLIENT_SETTINGS
#undef HPHPD_CLIENT_SETTING
  dummy(false) {}
};

#define DECLARE_DBG_CLIENT_SETTING                      \
  DebuggerClientSettings m_dbgClientSettings;           \

#define HPHPD_CLIENT_SETTING(name, type, defval)        \
type getDebuggerClient##name () const {                 \
  return m_dbgClientSettings.m_s##name;                 \
}                                                       \
void setDebuggerClient##name (type in##name) {          \
  m_dbgClientSettings.m_s##name = in##name;             \
}                                                       \


#define DECLARE_DBG_CLIENT_SETTING_ACCESSORS            \
HPHPD_CLIENT_SETTINGS

// leaving HPHPD_CLIENT_SETTING defined so that DECLARE_DBG_SETTING_ACCESSORS is
// effective

///////////////////////////////////////////////////////////////////////////////

#endif // __HPHP_EVAL_DEBUGGER_CLIENT_SETTINGS_H__
