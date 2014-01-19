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

#ifndef incl_HPHP_INI_SETTING_H_
#define incl_HPHP_INI_SETTING_H_

#include "hphp/runtime/base/complex-types.h"

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

  typedef bool (*UpdateCallback)(const String& value, void *p);
  typedef String (*GetCallback)(void *p);

public:
  static Variant FromString(const String& ini, const String& filename,
                            bool process_sections, int scanner_mode);

  static bool Get(const String& name, String &value);
  static bool Set(const String& name, const String& value);

  static void Bind(const char *name, const char *value,
                   UpdateCallback updateCallback, GetCallback getCallback,
                   void *p = nullptr);
  static void Unbind(const char *name);

  static void SetGlobalDefault(const char *name, const char *value);

};

bool ini_on_update_bool(const String& value, void *p);
bool ini_on_update_long(const String& value, void *p);
bool ini_on_update_non_negative(const String& value, void *p);
bool ini_on_update_real(const String& value, void *p);
bool ini_on_update_string(const String& value, void *p);
bool ini_on_update_string_non_empty(const String& value, void *p);

String ini_get_bool(void *p);
String ini_get_long(void *p);
String ini_get_real(void *p);
String ini_get_string(void *p);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_INI_SETTING_H_
