<?hh
// (c) Facebook, Inc. and its affiliates. Confidential and proprietary.
<<file: __EnableUnstableFeatures('readonly')>>

namespace HH\Readonly {

/**
 * Converts a readonly value type into a mutable one.
 * Value types include numerics, strings, bools, null and Hack arrays of value
 * types.
 */
<<__IgnoreReadonlyLocalErrors>>
function as_mut<T>(readonly T $x)[]: T {
  if (is_value_type(readonly $x)) {
    return $x;
  }
  throw new \InvalidArgumentException(__FUNCTION__.' expects a value type');
}

function is_value_type(readonly mixed $x)[]: bool {
  if ($x is string || $x is int || $x is float || $x is bool || $x is null) {
    return true;
  }
  if ($x is \HH\AnyArray) {
    foreach ($x as $v) {
      if (!is_value_type($v)) {
        return false;
      }
    }
    return true;
  }
  return false;
}

/**
 * Converts a readonly value to into a mutable one but omits validation for the
 * purposes of performance.
 * This function takes advantage of non-enforcement of readonly is SystemLib
 * and is not safe.
 */
<<__IgnoreReadonlyLocalErrors>>
function as_mut_without_validation<T>(readonly T $x)[]: T {
  return $x;
}

}
