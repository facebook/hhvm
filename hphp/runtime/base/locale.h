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

#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <locale.h>
#ifdef __APPLE__
#include <xlocale.h>
#endif

namespace HPHP {

/* Act on exactly 1 category, e.g. LC_CTYPE
 *
 * This is useful for interoperatbility with `setlocale()`.
 */
enum LocaleCategoryMode { LocaleCategory };
/* Act on multiple categories, e.g. LC_CTYPE_MASK | LC_TIME_MASK
 *
 * This is useful for interoperability with `newlocale()`
 */
enum LocaleCategoryMaskMode { LocaleCategoryMask };

/** This class lets us provide the benefits of both `setlocale()` and
 * `uselocale()`, and keep them compatible.
 *
 * - `setlocale()` uses and returns locale strings, but is per-process
 * - `uselocale()` is per thread, but is based on `locale_t` and there is not a
 *   standard way to get the locale string out of it
 *   - BSD/MacOS provide `querylocale()` which adds this functionality, but
 *     glibc does not.
 *
 * This class tracks mutations, applies them to a locale_t, and computes the
 * changes to the locale strings too, effectively emulating `querylocale()`.
 *
 * This uses std::string, std::vector, std::shared_ptr etc instead of HHVM's
 * usual abstractions as ThreadSafeSetLocale depends on it, and can be called
 * *very* early, e.g. from static initializers in random C libraries.
 */
struct Locale final {
  static std::shared_ptr<Locale> getCLocale();
  static std::shared_ptr<Locale> getEnvLocale();

  ~Locale();

  locale_t get() const;
  const char* querylocale(LocaleCategoryMode, int category); 
  const char* querylocale(LocaleCategoryMaskMode, int mask); 

  std::shared_ptr<Locale> newlocale(LocaleCategoryMode, int category, const char* locale);
  std::shared_ptr<Locale> newlocale(LocaleCategoryMaskMode, int mask, const char* locale);

  std::map<std::string, std::string> getAllCategoryLocaleNames();
private:
  struct CategoryAndLocaleRow final {
    int category;
    int category_mask;
    std::string category_str;
    std::string locale_str;
  };
  using CategoryAndLocaleMap = std::vector<CategoryAndLocaleRow>;
  
  locale_t m_locale;
  CategoryAndLocaleMap m_map;

  Locale(locale_t, const CategoryAndLocaleMap&);
};

} // namespace HPHP
