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

#include "hphp/runtime/base/locale.h"
#include "hphp/runtime/base/thread-safe-setlocale.h"
#include "hphp/util/assertions.h"

#include <string.h>
#include <dlfcn.h>
#include <langinfo.h>

namespace HPHP {

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

std::shared_ptr<Locale> Locale::getCLocale() {
  static std::shared_ptr<Locale> ret;
  if (ret) {
    return ret;
  }
  auto locale = ::newlocale(LC_ALL, "C", (locale_t) 0);

  CategoryAndLocaleMap map = {
#define CATEGORY_LOCALE_MAP_ENTRY(category) \
  {category, category ## _MASK, #category, "C"}
    FILL_IN_CATEGORY_LOCALE_MAP()
#undef CATEGORY_LOCALE_MAP_ENTRY
  };


  ret.reset(new Locale(locale, map));
  return ret;
}

std::shared_ptr<Locale> Locale::getEnvLocale() {
  static std::shared_ptr<Locale> ret;
  if (ret) {
    return ret;
  }
  auto locale = ::newlocale(LC_ALL, "", (locale_t) 0);
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

  CategoryAndLocaleMap map = {
#define CATEGORY_LOCALE_MAP_ENTRY(category) \
  {category, category ## _MASK, #category, strdup((*real_setlocale)(category, nullptr))}
    FILL_IN_CATEGORY_LOCALE_MAP()
#undef CATEGORY_LOCALE_MAP_ENTRY
  };

  ret.reset(new Locale(locale, map));
  return ret;
}

namespace {

Locale::CodesetKind infer_codeset_kind(locale_t loc) {
  const char* codeset = nl_langinfo_l(CODESET, loc);

  if (LIKELY(codeset && codeset[1] == 'C' && codeset[2] == 0)) {
    return Locale::CodesetKind::SINGLE_BYTE;
  }

  if (codeset && codeset[0]) {
    if (strcmp(codeset, "UTF-8") == 0 || strcmp(codeset, "utf8") == 0) {
      return Locale::CodesetKind::UTF8;
    }
    return Locale::CodesetKind::SINGLE_BYTE;
  }
#if defined(__APPLE__)
  // If you specify a locale without an encoding, the CODESET is always
  // null/empty on MacOS. MacOS is UTF-8 first, but uses single-byte for 'C'.
  //
  // To distinguish between these, we look at the max number of bytes in a
  // character
  if (MB_CUR_MAX_L(loc) <= 1) {
    return Locale::CodesetKind::SINGLE_BYTE;
  }
  return Locale::CodesetKind::UTF8;
#elif defined(__GLIBC__)
  // UTF-8 is always explicit, so if we don't have one, it's single-byte.
  return Locale::CodesetKind::SINGLE_BYTE;
#else
#error Unsupported platform
#endif
}

}// namespace

Locale::~Locale() {
  freelocale(m_locale);
}

locale_t Locale::get() const {
  return m_locale;
}

const char* Locale::querylocale(LocaleCategoryMode, int category) const {
  for (const auto& row: m_map) {
    if (row.category == category) {
      return row.locale_str.c_str();
    }
  }
  return nullptr;
}

const char* Locale::querylocale(LocaleCategoryMaskMode, int mask) const {
  for (const auto& row: m_map) {
    if ((row.category_mask & mask) == row.category_mask) {
      return row.locale_str.c_str();
    }
  }
  return nullptr;
}

std::shared_ptr<Locale> Locale::newlocale(LocaleCategoryMode,
                                          int category,
                                          const char* locale) const {
  for (const auto& row : m_map) {
    if (row.category == category) {
      return newlocale(LocaleCategoryMask, row.category_mask, locale);
    }
  }
  return nullptr;
}

std::shared_ptr<Locale> Locale::newlocale(LocaleCategoryMaskMode,
                                          int mask,
                                          const char* locale) const {
  auto env = getEnvLocale();

  auto names = m_map;
  for (auto& row : names) {
    if ((row.category_mask & mask) != row.category_mask) {
      continue;
    }
    row.locale_str = locale[0] == 0 ? env->querylocale(LocaleCategoryMask, row.category_mask) : locale;
  }

  // duplocale():
  // - we want to be able to call freelocale() eventually
  // - ::newlocale() invalidates the old locale
  //   - this can lead to a double-free if we don't call duplocale()
  //   - newlocale() isn't in-place or a copy: it invalidates the original when
  //     making a new one. This doesn't really fit with Hack's model, so let's
  //     just copy
  locale_t base_locale = m_locale ? duplocale(m_locale) : 0;
  locale_t new_locale = ::newlocale(mask, locale, base_locale);
  if (!new_locale) {
    if (base_locale) {
      freelocale(base_locale);
    }
    return nullptr;
  }
  auto codeset_kind = (mask & LC_CTYPE_MASK) == LC_CTYPE_MASK
    ? infer_codeset_kind(new_locale)
    : m_codesetKind;
  return std::shared_ptr<Locale>(new Locale(new_locale, names, codeset_kind));
}

std::map<std::string, std::string> Locale::getAllCategoryLocaleNames() {
  std::map<std::string, std::string> ret;
  for (const auto& row : m_map) {
    ret.emplace(row.category_str, row.locale_str);
  }
  return ret;
}

Locale::Locale(locale_t l,
               const CategoryAndLocaleMap& m
): m_locale(l), m_map(m), m_codesetKind(infer_codeset_kind(l)) {
  assertx(l);
}

Locale::Locale(locale_t l,
               const CategoryAndLocaleMap& m,
               CodesetKind ck): m_locale(l), m_map(m), m_codesetKind(ck) {
  assertx(l);
}

} // namespace HPHP
