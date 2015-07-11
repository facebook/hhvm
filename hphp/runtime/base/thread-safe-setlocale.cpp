/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/base/request-local.h"
#include "hphp/runtime/base/execution-context.h"

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

IMPLEMENT_THREAD_LOCAL(ThreadSafeLocaleHandler, g_thread_safe_locale_handler);
IMPLEMENT_THREAD_LOCAL(struct lconv, g_thread_safe_localeconv_data);

static const locale_t s_null_locale = (locale_t) 0;

ThreadSafeLocaleHandler::ThreadSafeLocaleHandler() {
  m_category_locale_map = {
#define FILL_IN_CATEGORY_LOCALE_MAP(category) \
  {category, category ## _MASK, #category, ""}
      FILL_IN_CATEGORY_LOCALE_MAP(LC_CTYPE),
      FILL_IN_CATEGORY_LOCALE_MAP(LC_NUMERIC),
      FILL_IN_CATEGORY_LOCALE_MAP(LC_TIME),
      FILL_IN_CATEGORY_LOCALE_MAP(LC_COLLATE),
      FILL_IN_CATEGORY_LOCALE_MAP(LC_MONETARY),
      FILL_IN_CATEGORY_LOCALE_MAP(LC_MESSAGES),
      FILL_IN_CATEGORY_LOCALE_MAP(LC_ALL),
      #ifndef __APPLE__
      FILL_IN_CATEGORY_LOCALE_MAP(LC_PAPER),
      FILL_IN_CATEGORY_LOCALE_MAP(LC_NAME),
      FILL_IN_CATEGORY_LOCALE_MAP(LC_ADDRESS),
      FILL_IN_CATEGORY_LOCALE_MAP(LC_TELEPHONE),
      FILL_IN_CATEGORY_LOCALE_MAP(LC_MEASUREMENT),
      FILL_IN_CATEGORY_LOCALE_MAP(LC_IDENTIFICATION),
      #endif
    #undef FILL_IN_CATEGORY_LOCALE_MAP
  };

  m_locale = s_null_locale;
  reset();
}

ThreadSafeLocaleHandler::~ThreadSafeLocaleHandler() {
  reset();
}

void ThreadSafeLocaleHandler::reset() {
  if (m_locale != s_null_locale) {
    freelocale(m_locale);
    m_locale = s_null_locale;
  }

  uselocale(LC_GLOBAL_LOCALE);
}

const char* ThreadSafeLocaleHandler::actuallySetLocale(
  int category, const char* locale_cstr) {
  if (category < 0 || category > m_category_locale_map.size()) {
    return nullptr;
  }

  if (locale_cstr == nullptr) {
    if (category == LC_ALL) {
      generate_LC_ALL_String();
    }

    return m_category_locale_map[category].locale_str.c_str();
  }

  if (category != LC_ALL && strchr(locale_cstr, '=') != nullptr) {
    return nullptr;
  }

  locale_t new_locale = newlocale(
    m_category_locale_map[category].category_mask,
    locale_cstr,
    m_locale
  );

  if (new_locale == s_null_locale) {
    return nullptr;
  }

  m_locale = new_locale;
  uselocale(m_locale);

  if (category == LC_ALL) {
    if (strchr(locale_cstr, ';') != nullptr) {
      /*
       * We need to parse out any semi-colon delimited categories and locales
       * and store them separately in the appropriate
       * category_locale_map.locale_str
       *
       * We are not validating the string for correctness of format.
       * The newlocale() call already did that for us.
       */
      char *locale_cstr_copy = strdup(locale_cstr);

      for (char *start_ptr = locale_cstr_copy, *group_save = nullptr;;
           start_ptr = nullptr) {
        char *group = strtok_r(start_ptr, ";", &group_save);
        if (group == nullptr) {
          break;
        }

        char *key = nullptr, *value = nullptr;
        int count = 0;
        for (char *item_ptr = group, *item_save = nullptr;;
             count++, item_ptr = nullptr) {
          char *item = strtok_r(item_ptr, "=", &item_save);
          if (item == nullptr) {
            break;
          }

          if (count == 0) {
            key = item;
          } else if (count == 1) {
            value = item;
          }
        }

        /* Completely naive search.
         * If this is a bottleneck it can be converted into a hash-map
         */
        for (auto &i : m_category_locale_map) {
          if (i.category_str == key) {
            i.locale_str = value;
            break;
          }
        }
      }

      free(locale_cstr_copy);
    } else {
      /* Copy the locale into all categories */
      for (auto &i : m_category_locale_map) {
        i.locale_str = locale_cstr;
      }
    }
  } else {
    m_category_locale_map[category].locale_str = locale_cstr;
  }

  return locale_cstr;
}

#ifdef __APPLE__
struct lconv* ThreadSafeLocaleHandler::localeconv() {
  // BSD/OS X has localeconv_l, which actually returns data held onto by the
  // locale itself -- and since that's thread-local (since this object instance
  // is) we can just use that.
  // TODO is the memcpy even necessary?
  struct lconv *ptr = g_thread_safe_localeconv_data.get();
  struct lconv *l = localeconv_l(m_locale);
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
  bool same = true;
  for (auto &i : m_category_locale_map) {
    if (i.category == LC_ALL) {
      continue;
    }

    if (i.locale_str != m_category_locale_map[LC_CTYPE].locale_str) {
      same = false;
      break;
    }
  }

  auto &all_locale_str = m_category_locale_map[LC_ALL].locale_str;
  if (same) {
    all_locale_str = m_category_locale_map[LC_CTYPE].locale_str;
  } else {
    all_locale_str.clear();

    for (auto &i : m_category_locale_map) {
      if (i.category == LC_ALL) {
        continue;
      }

      all_locale_str.append(i.category_str + "=" + i.locale_str + ";");
    }

    /* Remove trailing semicolon */
    all_locale_str.resize(all_locale_str.size() - 1);
  }
}

}

extern "C" char* setlocale(int category, const char* locale) {
  /* The returned char* is exactly what was passed in. */
  return const_cast<char*>
    (HPHP::g_thread_safe_locale_handler->actuallySetLocale(category, locale));
}

extern "C" struct lconv* localeconv() {
  return HPHP::g_thread_safe_locale_handler->localeconv();
}
