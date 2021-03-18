<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 *
 */

namespace HH\Lib\Dict;

/**
 * Casts the given traversable to a dict, resetting the legacy array mark
 * if applicable.
 */
function cast_clear_legacy_array_mark<Tk as arraykey, Tv>(
  KeyedTraversable<Tk, Tv> $x,
)[]: dict<Tk, Tv> {
  return \HH\is_dict_or_darray($x)
    /* HH_FIXME[4259] Rx doesn't understand array_unmark_legacy */
    ? dict(\HH\array_unmark_legacy($x))
    : dict($x);
}
