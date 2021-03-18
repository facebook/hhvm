<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 *
 */

namespace HH\Lib\Str;

use namespace HH\Lib\Vec;

/**
 * Returns a string formed by joining the elements of the Traversable with the
 * given `$glue` string.
 *
 * Previously known as `implode` in PHP.
 */
function join(
  Traversable<arraykey> $pieces,
  string $glue,
)[]: string {
  if ($pieces is Container<_>) {
    /* HH_FIXME[2049] __PHPStdLib */
    /* HH_FIXME[4107] __PHPStdLib */
    return \implode($glue, $pieces);
  }
  /* HH_FIXME[2049] __PHPStdLib */
  /* HH_FIXME[4107] __PHPStdLib */
  return \implode($glue, Vec\cast_clear_legacy_array_mark($pieces));
}
