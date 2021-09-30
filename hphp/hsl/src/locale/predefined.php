<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 *
 */

namespace HH\Lib\Locale;

use namespace HH\Lib\_Private\_Locale;

/** DEPRECATED: Use `Locale\bytes()` instead.
 *
 * This function is being removed as:
 * - there is often confusion between "the C locale" and
 *   "the current libc locale"
 * - HHVM implements optimizations which are a visible behavior change; for
 *   example, `strlen("foo\0bar")` is 7 in HHVM, but 3 in libc.
 */
function c()[]: Locale {
  return _Locale\get_c_locale();
}

/** Retrieve a fixed locale suitable for byte-based operations.
 *
 * This is similar to the "C" locale, also known as the "POSIX" or "en_US_POSIX"
 * locale; it does not vary based on user/environment/machine settings.
 *
 * It differs from the real "C" locale in that it is usable on strings that
 * contain null bytes; for example, `Str\length_l(Locale\bytes(), "foo\0bar")`
 * will return 7, instead of 3. The behavior is equivalent if the strings
 * are well-formed.
 */
function bytes()[]: Locale {
  return _Locale\get_c_locale();
}

/** Retrieve the locale being used by libc functions for the current thread.
 *
 * In general, we discourage this: it can be surprising that it changes the
 * behavior of many libc functions, like `sprintf('%f'`), and error messages
 * from native code may be translated.
 *
 * For web applications, that's likely unwanted - we recommend frameworks add
 * the concept of a 'viewer locale', and explicitly pass it to the relevant
 * string functions instead.
 *
 * @see `set_native()`
 */
function get_native(): Locale {
  return _Locale\get_request_locale();
}

/** Set the libc locale for the current thread.
 *
 * This is highly discouraged; see the note for `get_native()` for details.
 */
function set_native(Locale $loc): void {
  _Locale\set_request_locale($loc);
}

/** Retrieve the active locale from the native environment.
 *
 * This is usually set based on the `LC_*` environment variables.
 *
 * Web applications targeting diverse users should probably not use this,
 * however it is useful when aiming to support diverse users in CLI
 * programs.
 */
function from_environment(): Locale {
  return _Locale\get_environment_locale();
}
