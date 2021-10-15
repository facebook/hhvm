<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the hphp/hsl/ subdirectory of this source tree.
 *
 */

namespace HH\Lib\Str;

use namespace HH\Lib\{Locale, _Private\_Str};

/**
 * Returns a vec containing the string split into chunks with the given number
 * of characters.
 *
 * $chunk_size is in characters,
 *
 * To split the string on a delimiter, see `Str\split_l()`.
 */
function chunk_l(
  Locale\Locale $locale,
  string $string,
  int $chunk_size = 1,
)[]: vec<string> {
  return _Str\chunk_l($string, $chunk_size, $locale);
}

/**
 * Returns a vec containing the string split on the given delimiter. The vec
 * will not contain the delimiter itself.
 *
 * If the limit is provided, the vec will only contain that many elements, where
 * the last element is the remainder of the string.
 *
 * To split the string into equally-sized chunks, see `Str\chunk_l()`.
 * To use a pattern as delimiter, see `Regex\split()`.
 */
function split_l(
  Locale\Locale $locale,
  string $string,
  string $delimiter,
  ?int $limit = null,
)[]: vec<string> {
  return _Str\split_l($string, $delimiter, $limit, $locale);
}
