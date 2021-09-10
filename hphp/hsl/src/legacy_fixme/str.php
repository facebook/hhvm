<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the hphp/hsl/ subdirectory of this source tree.
 *
 */

namespace HH\Lib\Legacy_FIXME;
use namespace HH\Lib\{C, Dict, Str, Vec};

/** Fix invalid inputs to `Str\replace_every` and similar functions.
 *
 * Replacement pairs are required to be a string-to-string map, where the key
 * is a non-empty string (as find-replace for the empty string doesn't make
 * sense).
 *
 * Previously, these requirements were not consistently enforced; the HSL would
 * sometimes raise an error, but sometimes would coerce to string, and silently
 * drop empty string keys.
 *
 * Non-string keys/values required a FIXME.
 *
 * This function is intended to be used like so:
 *
 *    $out = Str\replace_every(
 *      $in,
 *      Legacy_FIXME\coerce_possibly_invalid_str_replace_pairs($replacements)
 *    );
 *
 * Calls to this function should be removed when safe to do so.
 */
function coerce_possibly_invalid_str_replace_pairs(
  KeyedContainer<string, string> $pairs,
)[]: dict<string, string> {
  return $pairs
    |> Dict\pull_with_key($$, ($_k, $v) ==> (string) $v, ($k, $_v) ==> (string) $k)
    |> Dict\filter_keys($$, $k ==> $k !== '');
}

/** `Str\split()`, with old behavior for negative limits.
 *
 * `Str\split()` now consistently bans negative limits.
 *
 * Previously, negative limits were banned if the delimiter were the empty
 * string, but other delimiters would lead to truncation - unlike positive
 * limits, which lead to concatenation.
 *
 * For example:

 *   Str\split('a!b!c', '!') === vec['a', 'b', c']
 *   Str\split('a!b!c', '!', 2) === vec['a', 'b!c']
 *   Str\split('a!b!c', '!', -1) === vec['a', 'b']
 *
 *
 * This function reimplements this old behavior; `Str\split()` will now
 * consistently throw on negative limits.
 */
function split_with_possibly_negative_limit(
  string $string,
  string $delimiter,
  ?int $limit = null,
)[]: vec<string> {
 if ($delimiter !== '' && $limit is int && $limit < 0) {
   $full = Str\split($string, $delimiter);
   $limit += C\count($full);
   if ($limit <= 0) {
     return vec[];
   }
   return Vec\take($full, $limit);
 }
 return Str\split(
   $string,
   $delimiter,
   $limit,
  );
}
