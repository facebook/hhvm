/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

TypedValue HHVM_FUNCTION(preg_match,
                         StringArg pattern, StringArg subject,
                         OutputArg matches /* = null */,
                         int flags /* = 0 */, int offset /* = 0 */) {
  return tvReturn(preg_match(StrNR(pattern.get()), StrNR(subject.get()),
                             matches.get() ? matches->var() : nullptr,
                             flags, offset));
}

TypedValue HHVM_FUNCTION(preg_match_all,
                         const String& pattern,
                         const String& subject,
                         OutputArg matches /* = null */,
                         int flags /* = 0 */,
                         int offset /* = 0 */) {
  return tvReturn(preg_match_all(pattern, subject,
                                 matches.get() ? matches->var() : nullptr,
                                 flags, offset));
}

///////////////////////////////////////////////////////////////////////////////


Variant HHVM_FUNCTION(preg_replace, const Variant& pattern, const Variant& replacement,
                                    const Variant& subject, int limit /* = -1 */,
                                    VRefParam count /* = null */) {
  return preg_replace_impl(pattern, replacement, subject,
                           limit, count.getVariantOrNull(), false, false);
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
                           limit, count.getVariantOrNull(), true, false);
}

static Variant preg_replace_callback_array_impl(
  const Variant& patterns_and_callbacks,
  const Array& subjects,
  int limit,
  VRefParam count) {

  Array ret = Array::Create();
  auto key = 0;
  auto total_replacement_count = 0;
  for (ArrayIter s_iter(subjects); s_iter; ++s_iter) {
    assert(s_iter.second().isString());
    auto subj = s_iter.second();
    for (ArrayIter pc_iter(patterns_and_callbacks.toArray());
                           pc_iter; ++pc_iter) {
      Variant pattern(pc_iter.first());
      assert(pattern.isString());
      Variant callback(pc_iter.second());
      subj = HHVM_FN(preg_replace_callback)(pattern, callback, subj, limit,
                                            count);
      // If we got an error on the replacement, the subject will be null,
      // and then we will return null.
      if (subj.isNull()) {
        return init_null();
      }

      if (count.isReferenced()) {
        total_replacement_count += count.toInt64();
      }
    }
    ret.add(key++, subj);
  }

  // If count was passed in as an explicit reference, we will assign it to our
  // total replacement count; otherwise, count will just remained unassigned
  count.assignIfRef(total_replacement_count);

  // If there were no replacements (i.e., matches) return original subject(s)
  if (ret.empty()) {
    return subjects;
  }
  return ret;
}

Variant HHVM_FUNCTION(preg_replace_callback_array,
                      const Variant& patterns_and_callbacks,
                      const Variant& subject,
                      int limit /* = -1 */,
                      VRefParam count /* = uninit_null() */) {
  if (!patterns_and_callbacks.isArray()) {
    raise_warning(
      "%s() expects parameter 1 to be an array, %s given",
      __FUNCTION__+2 /* +2 removes the "f_" prefix */,
      getDataTypeString(patterns_and_callbacks.getType()).c_str()
    );
    return init_null();
  }

  // Now see if we need to raise any warnings because of not having a
  // valid callback function
  for (ArrayIter iter(patterns_and_callbacks.toArray()); iter; ++iter) {
    if (!is_callable(iter.second())) {
      raise_warning("Not a valid callback function %s",
                    iter.second().toString().data());
      return subject.isString() ? empty_string_variant()
                                : Variant(empty_array());
    }
  }

  if (subject.isString()) {
    Array subject_arr = Array::Create();
    subject_arr.add(0, subject.toString());
    Variant ret = preg_replace_callback_array_impl(
      patterns_and_callbacks, subject_arr, limit, count
    );
    // ret[0] could be an empty string
    return ret.isArray() ? ret.toArray()[0] : init_null();
  } else if (subject.isArray()) {
    return preg_replace_callback_array_impl(
      patterns_and_callbacks, subject.toArray(), limit, count
    );
  } else {
    // No warning is given here, just return null
    return init_null();
  }
}

Variant HHVM_FUNCTION(preg_filter, const Variant& pattern, const Variant& callback,
                                   const Variant& subject, int limit /* = -1 */,
                                   VRefParam count /* = null */) {
  return preg_replace_impl(pattern, callback, subject,
                           limit, count.getVariantOrNull(), false, true);
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

extern IMPLEMENT_THREAD_LOCAL(PCREglobals, tl_pcre_globals);

class PcreExtension final : public Extension {
public:
  PcreExtension() : Extension("pcre", NO_EXTENSION_VERSION_YET) {}

  void moduleInit() override {
    HHVM_RC_STR(PCRE_VERSION, pcre_version());

    HHVM_RC_INT(PREG_NO_ERROR, PHP_PCRE_NO_ERROR);
    HHVM_RC_INT(PREG_INTERNAL_ERROR, PHP_PCRE_INTERNAL_ERROR);
    HHVM_RC_INT(PREG_BACKTRACK_LIMIT_ERROR, PHP_PCRE_BACKTRACK_LIMIT_ERROR);
    HHVM_RC_INT(PREG_RECURSION_LIMIT_ERROR, PHP_PCRE_RECURSION_LIMIT_ERROR);
    HHVM_RC_INT(PREG_BAD_UTF8_ERROR, PHP_PCRE_BAD_UTF8_ERROR);
    HHVM_RC_INT(PREG_BAD_UTF8_OFFSET_ERROR, PHP_PCRE_BAD_UTF8_OFFSET_ERROR);

    HHVM_RC_INT_SAME(PREG_PATTERN_ORDER);
    HHVM_RC_INT_SAME(PREG_SET_ORDER);
    HHVM_RC_INT_SAME(PREG_OFFSET_CAPTURE);

    HHVM_RC_INT_SAME(PREG_SPLIT_NO_EMPTY);
    HHVM_RC_INT_SAME(PREG_SPLIT_DELIM_CAPTURE);
    HHVM_RC_INT_SAME(PREG_SPLIT_OFFSET_CAPTURE);

    HHVM_RC_INT_SAME(PREG_GREP_INVERT);

#ifdef WNOHANG
    HHVM_RC_INT_SAME(WNOHANG);
#endif
#ifdef WUNTRACED
    HHVM_RC_INT_SAME(WUNTRACED);
#endif

#ifdef PRIO_PGRP
    HHVM_RC_INT_SAME(PRIO_PGRP);
#endif
#ifdef PRIO_USER
    HHVM_RC_INT_SAME(PRIO_USER);
#endif
#ifdef PRIO_PROCESS
    HHVM_RC_INT_SAME(PRIO_PROCESS);
#endif

    HHVM_FE(preg_filter);
    HHVM_FE(preg_grep);
    HHVM_FE(preg_match);
    HHVM_FE(preg_match_all);
    HHVM_FE(preg_replace);
    HHVM_FE(preg_replace_callback);
    HHVM_FE(preg_replace_callback_array);
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
