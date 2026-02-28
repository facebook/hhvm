<?hh

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

namespace HH\Lib\_Private\_Locale {

<<__NativeData>>
final class Locale {
  private function __construct() {}
  <<__Native>>
  public function __debugInfo(): dict<string, string>;
}

<<__Native>>
function get_c_locale()[]: Locale;
<<__Native>>
function get_environment_locale()[read_globals]: Locale;
<<__Native>>
function get_request_locale()[read_globals]: Locale;
<<__Native>>
function set_request_locale(Locale $loc)[globals]: void;

/** Behaves like `newlocale()`, taking a mask of categories, e.g. LC_CTYPE_MASK */
<<__Native>>
function newlocale_mask(int $mask, string $locale, Locale $base)[read_globals]: Locale;
/** Take a single category, e.g. `LC_TYPE` */
<<__Native>>
function newlocale_category(int $category, string $locale, Locale $base)[read_globals]: Locale;
/** Create a new locale object using the specified locale for all categories.
 *
 * This function will throw if a 'magic' locale is passed, e.g.
 * `"0"` (fetch current locale) or `""` (environment locale).
 */
<<__Native>>
function newlocale_all(string $locale)[]: Locale;

} // namespace _Locale

namespace HH\Lib\Locale {
  final class InvalidLocaleException extends \InvalidArgumentException {
  }
}
