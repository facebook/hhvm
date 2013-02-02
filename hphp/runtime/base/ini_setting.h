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

#ifndef __INI_SETTING_H__
#define __INI_SETTING_H__

#include <runtime/base/complex_types.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class IniSetting {
public:
  enum ScannerMode {
    NormalScanner,
    RawScanner,
  };

  enum ParserCallbackType {
    ParserSection,
    ParserEntry,
    ParserPopEntry,
  };

  typedef void (*PFN_PARSER_CALLBACK)(String *arg1, String *arg2, String *arg3,
                                      int callback_type, void *arg);

  typedef bool (*PFN_UPDATE_CALLBACK)(CStrRef value, void *p);

public:
  static Variant FromString(CStrRef ini, CStrRef filename,
                            bool process_sections, int scanner_mode);

  static bool Get(CStrRef name, String &value);
  static bool Set(CStrRef name, CStrRef value);

  static void Bind(const char *name, const char *value,
                   PFN_UPDATE_CALLBACK callback, void *p = NULL);
  static void Unbind(const char *name);

  static void SetGlobalDefault(const char *name, const char *value);

};

bool ini_on_update_bool(CStrRef value, void *p);
bool ini_on_update_long(CStrRef value, void *p);
bool ini_on_update_non_negative(CStrRef value, void *p);
bool ini_on_update_real(CStrRef value, void *p);
bool ini_on_update_string(CStrRef value, void *p);
bool ini_on_update_string_non_empty(CStrRef value, void *p);

///////////////////////////////////////////////////////////////////////////////
}

#endif // __INI_SETTING_H__
