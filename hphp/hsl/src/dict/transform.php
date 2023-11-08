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

use namespace HH\Lib\Math;

/**
 * Returns a vec containing the original dict split into chunks of the given
 * size.
 *
 * If the original dict doesn't divide evenly, the final chunk will be
 * smaller.
 *
 * Time complexity: O(n)
 * Space complexity: O(n)
 */
<<__NoAutoLikes>>
function chunk<Tk as arraykey, Tv>(
  KeyedTraversable<Tk, Tv> $traversable,
  int $size,
)[]: vec<dict<Tk, Tv>> {
  invariant($size > 0, 'Expected positive chunk size, got %d.', $size);
  $result = vec[];
  $ii = 0;
  $chunk_number = -1;
  foreach ($traversable as $key => $value) {
    if ($ii % $size === 0) {
      $result[] = dict[];
      $chunk_number++;
    }
    $result[$chunk_number][$key] = $value;
    $ii++;
  }
  return $result;
}

/**
 * Returns a new dict mapping each value to the number of times it appears
 * in the given Traversable.
 *
 * Time complexity: O(n)
 * Space complexity: O(n)
 */
function count_values<Tv as arraykey>(
  Traversable<Tv> $values,
)[]: dict<Tv, int> {
  $result = dict[];
  foreach ($values as $value) {
    $result[$value] = idx($result, $value, 0) + 1;
  }
  return $result;
}

/**
 * Returns a new dict formed by merging the KeyedContainer elements of the
 * given Traversable.
 *
 * In the case of duplicate keys, later values will overwrite
 * the previous ones.
 *
 * For a fixed number of KeyedTraversables, see `Dict\merge()`.
 *
 * Time complexity: O(n), where n is the combined size of all the
 * `$traversables`
 * Space complexity: O(n), where n is the combined size of all the
 * `$traversables`
 */
<<__NoAutoLikes>>
function flatten<Tk as arraykey, Tv>(
  Traversable<KeyedContainer<Tk, Tv>> $keyed_containers,
)[]: dict<Tk, Tv> {
  $result = dict[];
  foreach ($keyed_containers as $keyed_container) {
    if (!$result) {
      $result = dict($keyed_container);
    } else {
      foreach ($keyed_container as $key => $value) {
        $result[$key] = $value;
      }
    }
  }
  return $result;
}

/**
 * Returns a new dict where all the given keys map to the given value.
 *
 * Time complexity: O(n)
 * Space complexity: O(n)
 */
<<__NoAutoLikes>>
function fill_keys<Tk as arraykey, Tv>(
  Traversable<Tk> $keys,
  Tv $value,
)[]: dict<Tk, Tv> {
  $result = dict[];
  foreach ($keys as $key) {
    $result[$key] = $value;
  }
  return $result;
}

/**
 * Returns a new dict keyed by the values of the given KeyedTraversable
 * and vice-versa.
 *
 * In case of duplicate values, later keys overwrite the
 * previous ones.
 *
 * Time complexity: O(n)
 * Space complexity: O(n)
 */
<<__NoAutoLikes>>
function flip<Tk, Tv as arraykey>(
  KeyedTraversable<Tk, Tv> $traversable,
)[]: dict<Tv, Tk> {
  $result = dict[];
  foreach ($traversable as $key => $value) {
    $result[$value] = $key;
  }
  return $result;
}

/**
 * Returns a new dict where each value is the result of calling the given
 * function on the corresponding key.
 *
 * - To use an async function, see `Dict\from_key_async()`.
 * - To create a dict from values, see `Dict\from_values()`.
 * - To create a dict from key/value tuples, see `Dict\from_entries()`.
 *
 * Time complexity: O(n * f), where f is the complexity of `$value_func`
 * Space complexity: O(n)
 */
function from_keys<Tk as arraykey, Tv>(
  Traversable<Tk> $keys,
  (function(Tk)[_]: Tv) $value_func,
)[ctx $value_func]: dict<Tk, Tv> {
  $result = dict[];
  foreach ($keys as $key) {
    $result[$key] = $value_func($key);
  }
  return $result;
}

/**
 * Returns a new dict where each mapping is defined by the given key/value
 * tuples.
 *
 * In the case of duplicate keys, later values will overwrite the
 * previous ones.
 *
 * - To create a dict from keys, see `Dict\from_keys()`.
 * - To create a dict from values, see `Dict\from_values()`.
 *
 * Also known as `unzip` or `fromItems` in other implementations.
 *
 * Time complexity: O(n)
 * Space complexity: O(n)
 */
<<__NoAutoLikes>>
function from_entries<Tk as arraykey, Tv>(
  Traversable<(Tk, Tv)> $entries,
)[]: dict<Tk, Tv> {
  $result = dict[];
  foreach ($entries as list($key, $value)) {
    $result[$key] = $value;
  }
  return $result;
}

/**
 * Returns a new dict keyed by the result of calling the given function on each
 * corresponding value.
 *
 * In the case of duplicate keys, later values will
 * overwrite the previous ones.
 *
 * - To create a dict from keys, see `Dict\from_keys()`.
 * - To create a dict from key/value tuples, see `Dict\from_entries()`.
 * - To create a dict containing all values with the same keys, see `Dict\group_by()`.
 *
 * Time complexity: O(n)
 * Space complexity: O(n)
 */
function from_values<Tk as arraykey, Tv>(
  Traversable<Tv> $values,
  (function(Tv)[_]: Tk) $key_func,
)[ctx $key_func]: dict<Tk, Tv> {
  $result = dict[];
  foreach ($values as $value) {
    $result[$key_func($value)] = $value;
  }
  return $result;
}

 /**
  * Return a dict keyed by the result of calling the giving function, preserving
  * duplicate values.
  *
  *  - keys are the results of the given function called on the given values.
  *  - values are vecs of original values that all produced the same key.
  *
  * If a value produces a null key, it's omitted from the result.
  *
  * Time complexity: O(n * f), where f is the complexity of `$key_func`
  * Space complexity: O(n)
  */
function group_by<Tk as arraykey, Tv>(
  Traversable<Tv> $values,
  (function(Tv)[_]: ?Tk) $key_func,
)[ctx $key_func]: dict<Tk, vec<Tv>> {
  $result = dict[];
  foreach ($values as $value) {
    $key = $key_func($value);
    if ($key === null) {
      continue;
    }
    $result[$key] ??= vec[];
    $result[$key][] = $value;
  }
  return $result;
}

/**
 * Returns a new dict where each value is the result of calling the given
 * function on the original value.
 *
 * To use an async function, see `Dict\map_async()`.
 *
 * Time complexity: O(n * f), where f is the complexity of `$value_func`
 * Space complexity: O(n)
 */
function map<Tk as arraykey, Tv1, Tv2>(
  KeyedTraversable<Tk, Tv1> $traversable,
  (function(Tv1)[_]: Tv2) $value_func,
)[ctx $value_func]: dict<Tk, Tv2> {
  $result = dict[];
  foreach ($traversable as $key => $value) {
    $result[$key] = $value_func($value);
  }
  return $result;
}

/**
 * Returns a new dict where each key is the result of calling the given
 * function on the original key. In the case of duplicate keys, later values
 * will overwrite the previous ones.
 *
 * Time complexity: O(n * f), where f is the complexity of `$key_func`
 * Space complexity: O(n)
 */
function map_keys<Tk1, Tk2 as arraykey, Tv>(
  KeyedTraversable<Tk1, Tv> $traversable,
  (function(Tk1)[_]: Tk2) $key_func,
)[ctx $key_func]: dict<Tk2, Tv> {
  $result = dict[];
  foreach ($traversable as $key => $value) {
    $result[$key_func($key)] = $value;
  }
  return $result;
}

/**
 * Returns a new dict where each value is the result of calling the given
 * function on the original value and key.
 *
 * Time complexity: O(n * f), where f is the complexity of `$value_func`
 * Space complexity: O(n)
 */
function map_with_key<Tk as arraykey, Tv1, Tv2>(
  KeyedTraversable<Tk, Tv1> $traversable,
  (function(Tk, Tv1)[_]: Tv2) $value_func,
)[ctx $value_func]: dict<Tk, Tv2> {
  $result = dict[];
  foreach ($traversable as $key => $value) {
    $result[$key] = $value_func($key, $value);
  }
  return $result;
}

/**
 * Returns a new dict with mapped keys and values.
 *
 *  - values are the result of calling `$value_func` on the original value
 *  - keys are the result of calling `$key_func` on the original value.
 * In the case of duplicate keys, later values will overwrite the previous ones.
 *
 * Time complexity: O(n * (f1 + f2)), where f1 is the complexity of `$value_func`
 * and f2 is the complexity of `$key_func`
 * Space complexity: O(n)
 */
function pull<Tk as arraykey, Tv1, Tv2>(
  Traversable<Tv1> $traversable,
  (function(Tv1)[_]: Tv2) $value_func,
  (function(Tv1)[_]: Tk) $key_func,
)[ctx $value_func, ctx $key_func]: dict<Tk, Tv2> {
  $result = dict[];
  foreach ($traversable as $value) {
    $result[$key_func($value)] = $value_func($value);
  }
  return $result;
}

/**
 * Returns a new dict with mapped keys and values.
 *
 *  - values are the result of calling `$value_func` on the original value/key
 *  - keys are the result of calling `$key_func` on the original value/key.
 * In the case of duplicate keys, later values will overwrite the previous ones.
 *
 * Time complexity: O(n * (f1 + f2), where f1 is the complexity of `$value_func`
 * and f2 is the complexity of `$key_func`
 * Space complexity: O(n)
 */
function pull_with_key<Tk1, Tk2 as arraykey, Tv1, Tv2>(
  KeyedTraversable<Tk1, Tv1> $traversable,
  (function(Tk1, Tv1)[_]: Tv2) $value_func,
  (function(Tk1, Tv1)[_]: Tk2) $key_func,
)[ctx $value_func, ctx $key_func]: dict<Tk2, Tv2> {
  $result = dict[];
  foreach ($traversable as $key => $value) {
    $result[$key_func($key, $value)] = $value_func($key, $value);
  }
  return $result;
}
