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

/**
 * Create a new `Locale` object.
 *
 * The input should be of the form `country[.encoding]`, for example:
 * `"C"`, `en_US`, `en_US.UTF-8`.
 *
 * If present, the encoding currently **must** be 'UTF-8'.
 *
 * This will throw on 'magic' locales such as:
 * - the empty string: use `from_environment()`
 * - `'0'`: use `get_native()`
 */
function create(string $locale)[]: Locale {
  return _Locale\newlocale_all($locale);
}

/**
 * Create a new `Locale` object, based on an existing one.
 *
 * The input should be of the form `country[.encoding]`, for example:
 * `"C"`, `en_US`, `en_US.UTF-8`.
 *
 * If present, the encoding currently **must** be 'UTF-8'.
 *
 * The empty string is not considered a valid locale in Hack; the libc behavior
 * is equivalent to `get_native()`.
 */
function modified(Locale $orig, Category $cat, string $new)[read_globals]: Locale {
  if ($new === '') {
    // '' is the magic 'fetch from environment'
    throw new InvalidLocaleException(
      "Empty string passed; use `Locale\\from_environment() instead."
    );
  }

  return _Locale\newlocale_category(
    (int) $cat,
    $new,
    $orig,
  );
}
