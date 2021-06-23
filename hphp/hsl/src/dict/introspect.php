<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the hphp/hsl/ subdirectory of this source tree.
 *
 */

namespace HH\Lib\Dict;

use namespace HH\Lib\C;

/**
 * Returns whether the two given dicts have the same entries, using strict
 * equality. To guarantee equality of order as well as contents, use `===`.
 *
 * Time complexity: O(n)
 * Space complexity: O(1)
 */
function equal<Tk as arraykey, Tv>(
  dict<Tk, Tv> $dict1,
  dict<Tk, Tv> $dict2,
)[]: bool {
  if ($dict1 === $dict2) {
    return true;
  }
  if (C\count($dict1) !== C\count($dict2)) {
    return false;
  }
  foreach ($dict1 as $key => $value) {
    if (!C\contains_key($dict2, $key) || $dict2[$key] !== $value) {
      return false;
    }
  }
  return true;
}
