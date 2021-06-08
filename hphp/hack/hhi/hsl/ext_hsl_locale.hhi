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
}

function get_c_locale()[]: Locale;
function get_environment_locale(): Locale;
function get_request_locale(): Locale;
function set_request_locale(Locale $loc): void;

/** Behaves like `newlocale()`, taking a mask of categories, e.g. LC_CTYPE_MASK */
function newlocale_mask(int $mask, string $locale, Locale $base): Locale;
/** Take a single category, e.g. `LC_TYPE` */
function newlocale_category(int $category, string $locale, Locale $base): Locale;

// --- platform-specific constants ---
// more are defined for every platform, but the HHI is only including ones that
// are defined on every supported platform

const LC_ALL = 0xdeadbeef;
const LC_COLLATE = 0xdeadbeef;
const LC_CTYPE = 0xdeadbeef;
const LC_MONETARY = 0xdeadbeef;
const LC_NUMERIC = 0xdeadbeef;
const LC_TIME = 0xdeadbeef;
// required by POSIX, but not by C standard. Not supported by MSVC, but we don't
// support that (yet?).
const LC_MESSAGES = 0xdeadbeef;

const LC_ALL_MASK = 0xdeadbeef;
const LC_COLLATE_MASK = 0xdeadbeef;
const LC_CTYPE_MASK = 0xdeadbeef;
const LC_MONETARY_MASK = 0xdeadbeef;
const LC_NUMERIC_MASK = 0xdeadbeef;
const LC_TIME_MASK = 0xdeadbeef;
const LC_MESSAGES_MASK = 0xdeadbeef;

} // namespace HH\Lib\_Private\_Locale
