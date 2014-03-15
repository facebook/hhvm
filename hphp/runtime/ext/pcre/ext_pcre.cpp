/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/ext/pcre/ext_pcre.h"
#include "hphp/runtime/base/preg.h"

#include <pcre.h>

#include "hphp/runtime/ext/ext_mb.h"
#include "hphp/runtime/ext/ext_string.h"
#include "hphp/runtime/ext/ext_function.h"
#include "hphp/runtime/base/ini-setting.h"
#include "hphp/runtime/base/request-local.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

Variant HHVM_FUNCTION(preg_grep, const String& pattern, const Array& input,
                                 int flags /* = 0 */) {
  return preg_grep(pattern, input, flags);
}

///////////////////////////////////////////////////////////////////////////////

Variant HHVM_FUNCTION(preg_match, const String& pattern, const String& subject,
                                  VRefParam matches /* = null */,
                                  int flags /* = 0 */, int offset /* = 0 */) {
  if (matches.isReferenced()) {
    return preg_match(pattern, subject, matches, flags, offset);
  } else {
    return preg_match(pattern, subject, flags, offset);
  }
}

Variant HHVM_FUNCTION(preg_match_all, const String& pattern,
                                      const String& subject,
                                      VRefParam matches /* = null */,
                                      int flags /* = 0 */,
                                      int offset /* = 0 */) {
  if (matches.isReferenced()) {
    return preg_match_all(pattern, subject, matches, flags, offset);
  } else {
    return preg_match_all(pattern, subject, flags, offset);
  }
}

///////////////////////////////////////////////////////////////////////////////


Variant HHVM_FUNCTION(preg_replace, const Variant& pattern, const Variant& replacement,
                                    const Variant& subject, int limit /* = -1 */,
                                    VRefParam count /* = null */) {
  return preg_replace_impl(pattern, replacement, subject,
                           limit, count, false, false);
}

Variant HHVM_FUNCTION(preg_replace_callback, const Variant& pattern, const Variant& callback,
                                             const Variant& subject,
                                             int limit /* = -1 */,
                                             VRefParam count /* = null */) {
  if (!f_is_callable(callback)) {
    raise_warning("Not a valid callback function %s",
        callback.toString().data());
    return empty_string;
  }
  return preg_replace_impl(pattern, callback, subject,
                           limit, count, true, false);
}

Variant HHVM_FUNCTION(preg_filter, const Variant& pattern, const Variant& callback,
                                   const Variant& subject, int limit /* = -1 */,
                                   VRefParam count /* = null */) {
  return preg_replace_impl(pattern, callback, subject,
                           limit, count, false, true);
}

///////////////////////////////////////////////////////////////////////////////

Variant HHVM_FUNCTION(preg_split, const String& pattern, const String& subject,
                                  int limit /* = -1 */, int flags /* = 0 */) {
  return preg_split(pattern, subject, limit, flags);
}

///////////////////////////////////////////////////////////////////////////////

String HHVM_FUNCTION(preg_quote, const String& str,
                                 const Variant& delimiter /* = null_string */) {
  if (delimiter.isNull()) {
    return preg_quote(str, null_string);
  } else {
    return preg_quote(str, delimiter.toString());
  }
}

int64_t HHVM_FUNCTION(preg_last_error) {
  return preg_last_error();
}

///////////////////////////////////////////////////////////////////////////////
// ereg

String HHVM_FUNCTION(ereg_replace, const String& pattern,
                                   const String& replacement,
                                   const String& str) {
  return f_mb_ereg_replace(pattern, replacement, str);
}

String HHVM_FUNCTION(eregi_replace, const String& pattern,
                                    const String& replacement,
                                    const String& str) {
  return f_mb_eregi_replace(pattern, replacement, str);
}

Variant HHVM_FUNCTION(ereg, const String& pattern, const String& str,
                            VRefParam regs /* = null */) {
  return f_mb_ereg(pattern, str, ref(regs));
}

Variant HHVM_FUNCTION(eregi, const String& pattern, const String& str,
                             VRefParam regs /* = null */) {
  return f_mb_eregi(pattern, str, ref(regs));
}

///////////////////////////////////////////////////////////////////////////////
// regexec

Variant HHVM_FUNCTION(split, const String& pattern, const String& str,
                             int limit /* = -1 */) {
  return php_split(pattern, str, limit, false);
}

Variant HHVM_FUNCTION(spliti, const String& pattern, const String& str,
                              int limit /* = -1 */) {
  return php_split(pattern, str, limit, true);
}

String HHVM_FUNCTION(sql_regcase, const String& str) {
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

const StaticString s_PCRE_VERSION("PCRE_VERSION");

extern IMPLEMENT_THREAD_LOCAL(PCREglobals, s_pcre_globals);

class PcreExtension : public Extension {
public:
  PcreExtension() : Extension("pcre", NO_EXTENSION_VERSION_YET) {}

  virtual void moduleInit() {
    Native::registerConstant<KindOfString>(
      s_PCRE_VERSION.get(), makeStaticString(pcre_version())
    );

    HHVM_FE(preg_filter);
    HHVM_FE(preg_grep);
    HHVM_FE(preg_match);
    HHVM_FE(preg_match_all);
    HHVM_FE(preg_replace);
    HHVM_FE(preg_replace_callback);
    HHVM_FE(preg_split);
    HHVM_FE(preg_quote);
    HHVM_FE(preg_last_error);
    HHVM_FE(ereg_replace);
    HHVM_FE(eregi_replace);
    HHVM_FE(ereg);
    HHVM_FE(eregi);
    HHVM_FE(split);
    HHVM_FE(spliti);
    HHVM_FE(sql_regcase);

    loadSystemlib();
  }

  virtual void threadInit() {
    IniSetting::Bind(this, IniSetting::PHP_INI_ALL,
                     "pcre.backtrack_limit",
                     std::to_string(RuntimeOption::PregBacktraceLimit).c_str(),
                     &s_pcre_globals->m_preg_backtrace_limit);
    IniSetting::Bind(this, IniSetting::PHP_INI_ALL,
                     "pcre.recursion_limit",
                     std::to_string(RuntimeOption::PregRecursionLimit).c_str(),
                     &s_pcre_globals->m_preg_recursion_limit);
  }

} s_pcre_extension;

///////////////////////////////////////////////////////////////////////////////
}
