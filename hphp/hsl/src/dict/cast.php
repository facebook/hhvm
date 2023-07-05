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

/**
 * Casts the given traversable to a dict, resetting the legacy array mark
 * if applicable.
 */
<<__NoAutoLikes>>
function cast_clear_legacy_array_mark<Tk as arraykey, Tv>(
  KeyedTraversable<Tk, Tv> $x,
)[]: dict<Tk, Tv> {
  return ($x is dict<_,_>)
    ? dict(\HH\array_unmark_legacy($x))
    : dict($x);
}
