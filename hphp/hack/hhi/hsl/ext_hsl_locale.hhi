<?hh
/**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

namespace HH\Lib\Locale {
  final class InvalidLocaleException extends \Exception {}
}

namespace HH\Lib\_Private\_Locale {

  final class Locale {
    private function __construct() {}
    public function __debugInfo(): dict<arraykey, mixed>;
  }

  function get_c_locale()[]: Locale;
  function get_environment_locale()[read_globals]: Locale;
  function get_request_locale()[read_globals]: Locale;
  function set_request_locale(Locale $loc)[globals]: void;

  /** Behaves like `newlocale()`, taking a mask of categories, e.g. LC_CTYPE_MASK */
  function newlocale_mask(
    int $mask,
    string $locale,
    Locale $base,
  )[read_globals]: Locale;
  /** Take a single category, e.g. `LC_TYPE` */
  function newlocale_category(
    int $category,
    string $locale,
    Locale $base,
  )[read_globals]: Locale;
  /** Create a new locale object using the specified locale for all categories.
   *
   * This function will throw if a 'magic' locale is passed, e.g.
   * `"0"` (fetch current locale) or `""` (environment locale).
   */
  function newlocale_all(string $locale)[]: Locale;

  // --- platform-specific constants ---
  // more are defined for every platform, but the HHI is only including ones that
  // are defined on every supported platform

  const int LC_ALL;
  const int LC_COLLATE;
  const int LC_CTYPE;
  const int LC_MONETARY;
  const int LC_NUMERIC;
  const int LC_TIME;
  // required by POSIX, but not by C standard. Not supported by MSVC, but we don't
  // support that (yet?).
  const int LC_MESSAGES;

  const int LC_ALL_MASK;
  const int LC_COLLATE_MASK;
  const int LC_CTYPE_MASK;
  const int LC_MONETARY_MASK;
  const int LC_NUMERIC_MASK;
  const int LC_TIME_MASK;
  const int LC_MESSAGES_MASK;

} // namespace HH\Lib\_Private\_Locale
