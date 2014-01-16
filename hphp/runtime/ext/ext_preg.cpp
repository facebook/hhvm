/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#include "hphp/runtime/base/preg.h"

#include <pcre.h>

#include "hphp/runtime/ext/ext_mb.h"
#include "hphp/runtime/ext/ext_string.h"
#include "hphp/runtime/ext/ext_function.h"
#include "hphp/runtime/base/ini-setting.h"
#include "hphp/runtime/base/request-local.h"

namespace HPHP {

const StaticString s_PCRE_VERSION("PCRE_VERSION");

class PcreExtension : public Extension {
public:
  PcreExtension() : Extension("pcre") {}

  void moduleInit() override {
    Native::registerConstant<KindOfString>(
      s_PCRE_VERSION.get(), makeStaticString(pcre_version())
    );
  }
} s_pcre_extension;
///////////////////////////////////////////////////////////////////////////////

Variant f_preg_grep(const String& pattern, CArrRef input, int flags /* = 0 */) {
  return preg_grep(pattern, input, flags);
}

///////////////////////////////////////////////////////////////////////////////

Variant f_preg_match(const String& pattern, const String& subject,
                     VRefParam matches /* = null */, int flags /* = 0 */,
                     int offset /* = 0 */) {
  if (matches.isReferenced()) {
    return preg_match(pattern, subject, matches, flags, offset);
  } else {
    return preg_match(pattern, subject, flags, offset);
  }
}

Variant f_preg_match_all(const String& pattern, const String& subject,
                         VRefParam matches /* = null */, int flags /* = 0 */,
                         int offset /* = 0 */) {
  if (matches.isReferenced()) {
    return preg_match_all(pattern, subject, matches, flags, offset);
  } else {
    return preg_match_all(pattern, subject, flags, offset);
  }
}

///////////////////////////////////////////////////////////////////////////////


Variant f_preg_replace(CVarRef pattern, CVarRef replacement, CVarRef subject,
                       int limit /* = -1 */, VRefParam count /* = null */) {
  return preg_replace_impl(pattern, replacement, subject, limit, count, false);
}

Variant f_preg_replace_callback(CVarRef pattern, CVarRef callback,
                                CVarRef subject, int limit /* = -1 */,
                                VRefParam count /* = null */) {
  if (!f_is_callable(callback)) {
    raise_warning("Not a valid callback function %s",
        callback.toString().data());
    return empty_string;
  }
  return preg_replace_impl(pattern, callback, subject, limit, count, true);
}

///////////////////////////////////////////////////////////////////////////////

Variant f_preg_split(CVarRef pattern, CVarRef subject, int limit /* = -1 */,
                     int flags /* = 0 */) {
  return preg_split(pattern, subject, limit, flags);
}

///////////////////////////////////////////////////////////////////////////////

String f_preg_quote(const String& str, const String& delimiter /* = null_string */) {
  return preg_quote(str, delimiter);
}

int64_t f_preg_last_error() {
  return preg_last_error();
}

///////////////////////////////////////////////////////////////////////////////
// ereg

String f_ereg_replace(const String& pattern, const String& replacement, const String& str) {
  return f_mb_ereg_replace(pattern, replacement, str);
}

String f_eregi_replace(const String& pattern, const String& replacement, const String& str) {
  return f_mb_eregi_replace(pattern, replacement, str);
}

Variant f_ereg(const String& pattern, const String& str, VRefParam regs /* = null */) {
  return f_mb_ereg(pattern, str, ref(regs));
}

Variant f_eregi(const String& pattern, const String& str, VRefParam regs /* = null */) {
  return f_mb_eregi(pattern, str, ref(regs));
}

///////////////////////////////////////////////////////////////////////////////
// regexec

Variant f_split(const String& pattern, const String& str, int limit /* = -1 */) {
  return php_split(pattern, str, limit, false);
}

Variant f_spliti(const String& pattern, const String& str, int limit /* = -1 */) {
  return php_split(pattern, str, limit, true);
}

String f_sql_regcase(const String& str) {
  unsigned char c;
  register int i, j;

  char *tmp = (char*)malloc(str.size() * 4 + 1);
  for (i = j = 0; i < str.size(); i++) {
    c = (unsigned char)str.charAt(i);
    if (isalpha(c)) {
      tmp[j++] = '[';
      tmp[j++] = toupper(c);
      tmp[j++] = tolower(c);
      tmp[j++] = ']';
    } else {
      tmp[j++] = c;
    }
  }
  tmp[j] = 0;

  return String(tmp, j, AttachString);
}

///////////////////////////////////////////////////////////////////////////////
}
