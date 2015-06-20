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
#include "hphp/runtime/base/builtin-functions.h"

#include <pcre.h>

#include "hphp/runtime/ext/mbstring/ext_mbstring.h"
#include "hphp/runtime/ext/std/ext_std_function.h"
#include "hphp/runtime/ext/string/ext_string.h"
#include "hphp/runtime/base/ini-setting.h"
#include "hphp/runtime/base/request-local.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

static int s_pcre_has_jit = 0;

Variant HHVM_FUNCTION(preg_grep, const String& pattern, const Array& input,
                                 int flags /* = 0 */) {
  return preg_grep(pattern, input, flags);
}

///////////////////////////////////////////////////////////////////////////////

Variant HHVM_FUNCTION(preg_match,
                      const String& pattern, const String& subject,
                      VRefParam matches /* = null */,
                      int flags /* = 0 */, int offset /* = 0 */) {
  return preg_match(pattern, subject,
                    matches.isReferenced() ? &matches : nullptr,
                    flags, offset);
}

Variant HHVM_FUNCTION(preg_match_all,
                      const String& pattern,
                      const String& subject,
                      VRefParam matches /* = null */,
                      int flags /* = 0 */,
                      int offset /* = 0 */) {
  return preg_match_all(pattern, subject,
                        matches.isReferenced() ? &matches : nullptr,
                        flags, offset);
}

///////////////////////////////////////////////////////////////////////////////


Variant HHVM_FUNCTION(preg_replace, const Variant& pattern, const Variant& replacement,
                                    const Variant& subject, int limit /* = -1 */,
                                    VRefParam count /* = null */) {
  return preg_replace_impl(pattern, replacement, subject,
                           limit, count, false, false);
}

Variant HHVM_FUNCTION(preg_replace_callback,
                      const Variant& pattern,
                      const Variant& callback,
                      const Variant& subject,
                      int limit /* = -1 */,
                      VRefParam count /* = null */) {
  if (!is_callable(callback)) {
    raise_warning("Not a valid callback function %s",
        callback.toString().data());
    return empty_string_variant();
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
  return HHVM_FN(mb_ereg_replace)(pattern, replacement, str);
}

String HHVM_FUNCTION(eregi_replace, const String& pattern,
                                    const String& replacement,
                                    const String& str) {
  return HHVM_FN(mb_eregi_replace)(pattern, replacement, str);
}

Variant HHVM_FUNCTION(ereg, const String& pattern, const String& str,
                            VRefParam regs /* = null */) {
  return HHVM_FN(mb_ereg)(pattern, str, ref(regs));
}

Variant HHVM_FUNCTION(eregi, const String& pattern, const String& str,
                             VRefParam regs /* = null */) {
  return HHVM_FN(mb_eregi)(pattern, str, ref(regs));
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
  StringBuffer out(str.size());
  for (int i = 0; i < str.size(); i++) {
    unsigned char c = (unsigned char)str.charAt(i);
    if (isalpha(c)) {
      out.append('[');
      out.append((unsigned char)toupper(c));
      out.append((unsigned char)tolower(c));
      out.append(']');
    } else {
      out.append(c);
    }
  }

  return out.detach();
}

///////////////////////////////////////////////////////////////////////////////

const StaticString s_PCRE_VERSION("PCRE_VERSION");

extern IMPLEMENT_THREAD_LOCAL(PCREglobals, tl_pcre_globals);

class PcreExtension final : public Extension {
public:
  PcreExtension() : Extension("pcre", NO_EXTENSION_VERSION_YET) {}

  void moduleInit() override {
    Native::registerConstant<KindOfString>(
      s_PCRE_VERSION.get(), makeStaticString(pcre_version())
    );

#define PCRECNS(c) Native::registerConstant<KindOfInt64> \
                    (makeStaticString("PREG_" #c), PHP_PCRE_##c);
    PCRECNS(NO_ERROR);
    PCRECNS(INTERNAL_ERROR);
    PCRECNS(BACKTRACK_LIMIT_ERROR);
    PCRECNS(RECURSION_LIMIT_ERROR);
    PCRECNS(BAD_UTF8_ERROR);
    PCRECNS(BAD_UTF8_OFFSET_ERROR);
#undef PCRECNS
#define PREGCNS(c) Native::registerConstant<KindOfInt64> \
                    (makeStaticString("PREG_" #c), PREG_##c);
    PREGCNS(PATTERN_ORDER);
    PREGCNS(SET_ORDER);
    PREGCNS(OFFSET_CAPTURE);

    PREGCNS(SPLIT_NO_EMPTY);
    PREGCNS(SPLIT_DELIM_CAPTURE);
    PREGCNS(SPLIT_OFFSET_CAPTURE);

    PREGCNS(GREP_INVERT);
#undef PREGCNS

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

    pcre_config(PCRE_CONFIG_JIT, &s_pcre_has_jit);
    IniSetting::Bind(this, IniSetting::PHP_INI_ONLY,
                     "hhvm.pcre.jit",
                     &s_pcre_has_jit);
  }

  void threadInit() override {
    IniSetting::Bind(this, IniSetting::PHP_INI_ALL,
                     "pcre.backtrack_limit",
                     std::to_string(RuntimeOption::PregBacktraceLimit).c_str(),
                     &tl_pcre_globals->preg_backtrace_limit);
    IniSetting::Bind(this, IniSetting::PHP_INI_ALL,
                     "pcre.recursion_limit",
                     std::to_string(RuntimeOption::PregRecursionLimit).c_str(),
                     &tl_pcre_globals->preg_recursion_limit);
  }

} s_pcre_extension;

///////////////////////////////////////////////////////////////////////////////
}
