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

/**
 * Does the PHP style behaviour for casting when doing a mathematical operation.
 * That happens under the following situations
 *   1. null converts to 0
 *   2. bool converts to 0/1
 *   3. numeric string converts to an int or double based on how the string looks.
 *   4. non-numeric string gets converted to 0
 *   5. resources get casted to int
 */
function cast_for_arithmetic(mixed $value)[]: dynamic {
  if ($value is null) {
    return 0;
  }
  if ($value is bool || $value is resource) {
    return (int)$value;
  }
  return $value is string ? \HH\str_to_numeric($value) ?? 0 : $value;
}

/**
 * Does the PHP style behaviour for casting when doing an exponentiation.
 * That happens under the following situations
 *   1. function pointers, and arrays get converted to 0
 *   2. see castForArithmatic
 */
function cast_for_exponent(mixed $value)[]: dynamic {
  if (\HH\is_class_meth($value)) {
    return $value;
  }
  if (\HH\is_fun($value) || $value is AnyArray<_, _>) {
    return 0;
  }
  return cast_for_arithmetic($value);
}
