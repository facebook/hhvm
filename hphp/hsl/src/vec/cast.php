<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the hphp/hsl/ subdirectory of this source tree.
 *
 */

namespace HH\Lib\Vec;

/**
 * Casts the given traversable to a vec, resetting the legacy array mark
 * if applicable.
 */
<<__NoAutoLikes>>
function cast_clear_legacy_array_mark<T>(
  Traversable<T> $x,
)[]: vec<T> {
  return ($x is vec<_>)
    ? vec(\HH\array_unmark_legacy($x))
    : vec($x);
}
