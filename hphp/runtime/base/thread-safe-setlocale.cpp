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

namespace {

static const locale_t s_null_locale = (locale_t) 0;

} // namespace

#if defined(_MSC_VER)
#define FILL_IN_CATEGORY_LOCALE_MAP() \
      CATEGORY_LOCALE_MAP_ENTRY(LC_CTYPE), \
      CATEGORY_LOCALE_MAP_ENTRY(LC_NUMERIC), \
      CATEGORY_LOCALE_MAP_ENTRY(LC_TIME), \
      CATEGORY_LOCALE_MAP_ENTRY(LC_COLLATE), \
      CATEGORY_LOCALE_MAP_ENTRY(LC_MONETARY), \
      CATEGORY_LOCALE_MAP_ENTRY(LC_ALL)
#elif defined(__APPLE__)
#define FILL_IN_CATEGORY_LOCALE_MAP() \
      CATEGORY_LOCALE_MAP_ENTRY(LC_CTYPE), \
      CATEGORY_LOCALE_MAP_ENTRY(LC_NUMERIC), \
      CATEGORY_LOCALE_MAP_ENTRY(LC_TIME), \
      CATEGORY_LOCALE_MAP_ENTRY(LC_COLLATE), \
      CATEGORY_LOCALE_MAP_ENTRY(LC_MONETARY), \
      CATEGORY_LOCALE_MAP_ENTRY(LC_MESSAGES), \
      CATEGORY_LOCALE_MAP_ENTRY(LC_ALL)
#elif defined(__GLIBC__)
#define FILL_IN_CATEGORY_LOCALE_MAP()  \
      CATEGORY_LOCALE_MAP_ENTRY(LC_CTYPE), \
      CATEGORY_LOCALE_MAP_ENTRY(LC_NUMERIC), \
      CATEGORY_LOCALE_MAP_ENTRY(LC_TIME), \
      CATEGORY_LOCALE_MAP_ENTRY(LC_COLLATE), \
      CATEGORY_LOCALE_MAP_ENTRY(LC_MONETARY), \
      CATEGORY_LOCALE_MAP_ENTRY(LC_MESSAGES), \
      CATEGORY_LOCALE_MAP_ENTRY(LC_ALL), \
      CATEGORY_LOCALE_MAP_ENTRY(LC_PAPER), \
      CATEGORY_LOCALE_MAP_ENTRY(LC_NAME), \
      CATEGORY_LOCALE_MAP_ENTRY(LC_ADDRESS), \
      CATEGORY_LOCALE_MAP_ENTRY(LC_TELEPHONE), \
      CATEGORY_LOCALE_MAP_ENTRY(LC_MEASUREMENT), \
      CATEGORY_LOCALE_MAP_ENTRY(LC_IDENTIFICATION)
#else
#error "Unsupported platform"
#endif

ThreadSafeLocaleHandler::LocaleInfo& ThreadSafeLocaleHandler::getCLocale() {
  static std::optional<ThreadSafeLocaleHandler::LocaleInfo> li;
  if (li) {
    return *li;
  }
  auto locale = newlocale(LC_ALL, "C", (locale_t) 0);

  std::vector<CategoryAndLocaleMap> category_map = {
#define CATEGORY_LOCALE_MAP_ENTRY(category) \
  {category, category ## _MASK, #category, "C"}
    FILL_IN_CATEGORY_LOCALE_MAP()
#undef CATEGORY_LOCALE_MAP_ENTRY
  };

  li = std::make_tuple(locale, category_map);
  return *li;
}

ThreadSafeLocaleHandler::LocaleInfo& ThreadSafeLocaleHandler::getEnvLocale() {
  static std::optional<ThreadSafeLocaleHandler::LocaleInfo> li;
  if (li) {
    return *li;
  }
  auto locale = newlocale(LC_ALL, "", (locale_t) 0);
  // Per POSIX, a processes' default locale is "C" until `setlocale()` is
  // called; to use env from environment, `setlocale(LC_ALL, "")` should
  // be called.
  //
  // Can't actually check that nothing in HHVM has set the locale yet, but
  // this is pretty close.
  //
  // Asserting as we restore it later. The alternative would be to store
  // the original locale for each category, but if we're setting process
  // locale somewhere else, this should be re-reviewed anyway
  auto real_setlocale = reinterpret_cast<decltype(&setlocale)>(dlsym(RTLD_NEXT, "setlocale"));
  assertx(!strcmp((*real_setlocale)(LC_ALL, nullptr), "C"));
  // ... but to find out what the env locale actually is, we need to use it
  (*real_setlocale)(LC_ALL, "");
  SCOPE_EXIT { (*real_setlocale)(LC_ALL, "C"); };

  std::vector<CategoryAndLocaleMap> map = {
#define CATEGORY_LOCALE_MAP_ENTRY(category) \
  {category, category ## _MASK, #category, strdup((*real_setlocale)(category, nullptr))}
    FILL_IN_CATEGORY_LOCALE_MAP()
#undef CATEGORY_LOCALE_MAP_ENTRY
  };

  li = std::make_tuple(locale, map);
  return *li;
}

ThreadSafeLocaleHandler::ThreadSafeLocaleHandler() {
  m_locale = s_null_locale;
  reset();
}

ThreadSafeLocaleHandler::~ThreadSafeLocaleHandler() {
  reset();
}

void ThreadSafeLocaleHandler::reset() {
  if (m_locale != s_null_locale) {
    freelocale(m_locale);
  }
  std::tie(m_locale, m_category_locale_map) = getCLocale();
  m_locale = duplocale(m_locale);
  uselocale(m_locale);
}

const char* ThreadSafeLocaleHandler::actuallySetLocale(
  int category, const char* locale_cstr) {
  if (category < 0 || category >= m_category_locale_map.size()) {
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

#ifdef _MSC_VER
  // Windows doesn't accept POSIX as a valid
  // locale, use C instead.
  if (!strcmp(locale_cstr, "POSIX"))
    locale_cstr = "C";

  if (::setlocale(category, locale_cstr) == nullptr)
    return nullptr;
#else
  // newlocale invalidates the old one, so let's make a copy
  auto base_locale = m_locale ? duplocale(m_locale) : 0;
  locale_t new_locale = newlocale(
    m_category_locale_map[category].category_mask,
    locale_cstr,
    base_locale
  );

  if (!new_locale) {
    if (base_locale) {
      freelocale(base_locale);
    }
    return nullptr;
  }

  uselocale(new_locale);
  freelocale(m_locale);
  m_locale = new_locale;
#endif

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
            i.locale_str = value ? value : "C";
            break;
          }
        }
      }

      free(locale_cstr_copy);
    } else if (locale_cstr[0] == 0) {
      auto& [_, env_map] = getEnvLocale();
      m_category_locale_map = env_map;
      // LC_CTYPE is arbitrary: as we've set LC_ALL, any works
      locale_cstr = env_map[LC_CTYPE].locale_str.c_str();
    } else {
      /* Copy the locale into all categories */
      for (auto &i : m_category_locale_map) {
        i.locale_str = locale_cstr;
      }
    }
  } else if (locale_cstr[0] == 0) {
    auto& [_, env_map] = getEnvLocale();
    auto& locale_str = env_map[category].locale_str;
    m_category_locale_map[category].locale_str = locale_str;
    locale_cstr = locale_str.c_str();
  } else {
    m_category_locale_map[category].locale_str = locale_cstr;
  }

  return locale_cstr;
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
