/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/base/thread-safe-setlocale.h"
#include <string.h>
#include <math.h>
#include "hphp/runtime/base/string-buffer.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/util/rds-local.h"

#include <dlfcn.h>
#ifdef __APPLE__
#include <xlocale.h>
#endif

#ifdef __APPLE__
// On MacOS we have and use localeconv_l, but we emulate it on other platforms
#include <locale.h>
#endif

/*
 * This class reimplements the setlocale() and localeconv() functions
 * in a thread-safe manner and provides stronger symbols for them than
 * would be available from the glibc shared library.
 * This is part of hphp/runtime/base so the symbols are always available
 * in the main binary and hence act as be a global replacement for calls
 * even from other shared objects like ICU/etc.
 *
 * setlocale(): The implementation uses newlocale() and uselocale().
 * Because these don't provide a human readable locale string as output,
 * the implementation maintains a thread-local object to persist the
 * currently active locale settings. This design has meant some duplication to
 * what glibc already does in parsing the string locales but this should be OK
 * because setlocale would never be called enough to be a bottleneck.
 * The object is hooked into hphp_session_init() and hphp_context_shutdown()
 * to be init'd and reset'd at the end of a request. I chose not to use a
 * RequestEventHandler because setlocale() may be called during static
 * variable initialization before any of the HHVM stuff like memory
 * managers, etc. are even loaded. This is also why I used
 * std::string instead of the HHVM provided type.
 *
 * localeconv(): A thread-local struct lconv is maintained and it is filled by
 * making multiple calls to nl_langinfo() which is thread-safe.
 */

namespace HPHP {

RDS_LOCAL(ThreadSafeLocaleHandler, g_thread_safe_locale_handler);
RDS_LOCAL(struct lconv, g_thread_safe_localeconv_data);

ThreadSafeLocaleHandler::ThreadSafeLocaleHandler() {
  reset();
}

ThreadSafeLocaleHandler::~ThreadSafeLocaleHandler() {
  reset();
}

void ThreadSafeLocaleHandler::reset() {
  auto next = Locale::getCLocale();
  uselocale(next->get());
  m_locale = next;
}

const char* ThreadSafeLocaleHandler::actuallySetLocale(
  int category, const char* locale_cstr) {
  if (category < 0) {
    return nullptr;
  }

  if (locale_cstr == nullptr) {
    if (category == LC_ALL) {
      generate_LC_ALL_String();
      return m_lc_all.c_str();
    }

    return m_locale->querylocale(LocaleCategory, category);
  }

  if (category != LC_ALL && strchr(locale_cstr, '=') != nullptr) {
    return nullptr;
  }

  auto new_locale = m_locale->newlocale(LocaleCategory, category, locale_cstr);
  if (!new_locale) {
    return nullptr;
  }

  if (!uselocale(new_locale->get())) {
    return nullptr;
  }

  m_locale = new_locale;

  return m_locale->querylocale(LocaleCategory, category);
}

#ifdef _MSC_VER
struct lconv* ThreadSafeLocaleHandler::localeconv() {
  // We've setup locales to be thread local, so this is no
  // problem at all.
  struct lconv *ptr = g_thread_safe_localeconv_data.get();
  struct lconv *l = ::localeconv();
  memcpy(ptr, l, sizeof(struct lconv));
  return ptr;
}
#elif defined(__APPLE__)
struct lconv* ThreadSafeLocaleHandler::localeconv() {
  // BSD/OS X has localeconv_l, which actually returns data held onto by the
  // locale itself -- and since that's thread-local (since this object instance
  // is) we can just use that.
  // TODO is the memcpy even necessary?
  struct lconv *ptr = g_thread_safe_localeconv_data.get();
  struct lconv *l = localeconv_l(m_locale->get());
  memcpy(ptr, l, sizeof(struct lconv));
  return ptr;
}
#else
struct lconv* ThreadSafeLocaleHandler::localeconv() {
  // glibc does not have localeconv_l, and so we need to do some shenanigans.
  struct lconv *ptr = g_thread_safe_localeconv_data.get();

  ptr->decimal_point = nl_langinfo(DECIMAL_POINT);
  ptr->thousands_sep = nl_langinfo(THOUSANDS_SEP);
  ptr->grouping = nl_langinfo(GROUPING);
  ptr->int_curr_symbol = nl_langinfo(INT_CURR_SYMBOL);
  ptr->currency_symbol = nl_langinfo(CURRENCY_SYMBOL);
  ptr->mon_decimal_point = nl_langinfo(MON_DECIMAL_POINT);
  ptr->mon_thousands_sep = nl_langinfo(MON_THOUSANDS_SEP);
  ptr->mon_grouping = nl_langinfo(MON_GROUPING);
  ptr->positive_sign = nl_langinfo(POSITIVE_SIGN);
  ptr->negative_sign = nl_langinfo(NEGATIVE_SIGN);
  ptr->int_frac_digits = nl_langinfo(INT_FRAC_DIGITS)[0];
  ptr->frac_digits = nl_langinfo(FRAC_DIGITS)[0];
  ptr->p_cs_precedes = nl_langinfo(P_CS_PRECEDES)[0];
  ptr->p_sep_by_space = nl_langinfo(P_SEP_BY_SPACE)[0];
  ptr->n_cs_precedes = nl_langinfo(N_CS_PRECEDES)[0];
  ptr->n_sep_by_space = nl_langinfo(N_SEP_BY_SPACE)[0];
  ptr->p_sign_posn = nl_langinfo(P_SIGN_POSN)[0];
  ptr->n_sign_posn = nl_langinfo(N_SIGN_POSN)[0];

  #ifdef __USE_ISOC99
  ptr->int_p_cs_precedes = nl_langinfo(INT_P_CS_PRECEDES)[0];
  ptr->int_p_sep_by_space = nl_langinfo(INT_P_SEP_BY_SPACE)[0];
  ptr->int_n_cs_precedes = nl_langinfo(INT_N_CS_PRECEDES)[0];
  ptr->int_n_sep_by_space = nl_langinfo(INT_N_SEP_BY_SPACE)[0];
  ptr->int_p_sign_posn = nl_langinfo(INT_P_SIGN_POSN)[0];
  ptr->int_n_sign_posn = nl_langinfo(INT_N_SIGN_POSN)[0];
  #else
  ptr->__int_p_cs_precedes = nl_langinfo(INT_P_CS_PRECEDES)[0];
  ptr->__int_p_sep_by_space = nl_langinfo(INT_P_SEP_BY_SPACE)[0];
  ptr->__int_n_cs_precedes = nl_langinfo(INT_N_CS_PRECEDES)[0];
  ptr->__int_n_sep_by_space = nl_langinfo(INT_N_SEP_BY_SPACE)[0];
  ptr->__int_p_sign_posn = nl_langinfo(INT_P_SIGN_POSN)[0];
  ptr->__int_n_sign_posn = nl_langinfo(INT_N_SIGN_POSN)[0];
  #endif

  return ptr;
}
#endif

void ThreadSafeLocaleHandler::generate_LC_ALL_String() {
  auto names = m_locale->getAllCategoryLocaleNames();

  bool same = true;
  // Arbitrary - if anything != CTYPE then they're not all the same
  const auto lc_type = names.at("LC_CTYPE");
  for (const auto& [category, name] : names) {
    if (category == "LC_ALL") {
      continue;
    }

    if (name != lc_type) {
      same = false;
      break;
    }
  }

  if (same) {
    m_lc_all = names.at("LC_ALL");
    return;
  }

  m_lc_all.clear();

  for (const auto &[category, name] : names) {
    if (category == "LC_ALL") {
      continue;
    }

    m_lc_all.append(category + "=" + name + ";");
  }

  /* Remove trailing semicolon */
  m_lc_all.resize(m_lc_all.size() - 1);
}

std::shared_ptr<Locale> ThreadSafeLocaleHandler::getRequestLocale() {
  return HPHP::g_thread_safe_locale_handler->m_locale;
}

void ThreadSafeLocaleHandler::setRequestLocale(std::shared_ptr<Locale> loc) {
  if (!uselocale(loc->get())) {
    return;
  }
  HPHP::g_thread_safe_locale_handler->m_locale = loc;
}

}

#ifndef _MSC_VER
extern "C" char* setlocale(int category, const char* locale) {
  /* The returned char* is exactly what was passed in. */
  return const_cast<char*>
    (HPHP::g_thread_safe_locale_handler->actuallySetLocale(category, locale));
}

extern "C" struct lconv* localeconv() {
  return HPHP::g_thread_safe_locale_handler->localeconv();
}
#endif
