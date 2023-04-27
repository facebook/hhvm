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

namespace HPHP {

/* Act on exactly 1 category, e.g. LC_CTYPE
 *
 * This is useful for interoperability with `setlocale()`.
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
  enum class CodesetKind{
    SINGLE_BYTE,
    // UTF-8 is the only multibyte encoding that is portably supported as a C
    // locale; GB 18030 (a variable-length Chinese standard Unicode encoding) is
    // also supported by glibc, but not on MacOS, and not on most Windows
    // systems.
    //
    // Encodings like UTF16 can not be supported as a C locale as they include
    // null bytes, so UTF16 strings can not be 'C strings' - but they can be
    // byte arrays.
    UTF8,
  };

  static std::shared_ptr<Locale> getCLocale();
  static std::shared_ptr<Locale> getEnvLocale();

  ~Locale();

  locale_t get() const;
  const char* querylocale(LocaleCategoryMode, int category) const;
  const char* querylocale(LocaleCategoryMaskMode, int mask) const;

  std::shared_ptr<Locale> newlocale(LocaleCategoryMode, int category, const char* locale) const;
  std::shared_ptr<Locale> newlocale(LocaleCategoryMaskMode, int mask, const char* locale) const;

  std::map<std::string, std::string> getAllCategoryLocaleNames();
  CodesetKind getCodesetKind() {return m_codesetKind; }
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
  CodesetKind m_codesetKind;

  Locale(locale_t, const CategoryAndLocaleMap&);
  Locale(locale_t, const CategoryAndLocaleMap&, CodesetKind);
};

} // namespace HPHP
