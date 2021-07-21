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

/**
 * Does the PHP style behaviour when doing an inc/dec operation.
 * Specially handles
 *   1. incrementing null
 *   2. inc/dec on empty and numeric strings
 *
 * If you're seeing this in your code, it was added due to an implicit
 * conversion that either is or will soon be banned.
 */
function increment(mixed $value)[]: dynamic {
  if ($value is null) {
    return 1;
  }
  if ($value is string) {
    if (\is_numeric($value)) {
      return \HH\str_to_numeric($value) as nonnull + 1;
    }
    if ($value === '') {
      return '1';
    }
  }
  $value as dynamic;
  ++$value;
  return $value;
}

/**
 * See docs on increment
 */
function decrement(mixed $value)[]: dynamic {
  if ($value is string) {
    if (\is_numeric($value)) {
      return \HH\str_to_numeric($value) as nonnull - 1;
    }
    if ($value === '') {
      return -1;
    }
  }

  $value as dynamic;
  --$value;
  return $value;
}
