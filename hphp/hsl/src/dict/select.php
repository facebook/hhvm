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

use namespace HH\Lib\C;

/**
 * Returns a new dict containing only the entries of the first KeyedTraversable
 * whose keys do not appear in any of the other ones.
 *
 * Time complexity: O(n + m), where n is size of `$first` and m is the combined
 * size of `$second` plus all the `...$rest`
 * Space complexity: O(n + m), where n is size of `$first` and m is the combined
 * size of `$second` plus all the `...$rest` -- note that this is bigger than
 * O(n)
 */
function diff_by_key<Tk1 as arraykey, Tk2 as arraykey, Tv>(
  KeyedTraversable<Tk1, Tv> $first,
  KeyedTraversable<Tk2, mixed> $second,
  KeyedContainer<Tk2, mixed> ...$rest
)[]: dict<Tk1, Tv> {
  if (!$first) {
    return dict[];
  }
  if (!$second && !$rest) {
    return cast_clear_legacy_array_mark($first);
  }
  $union = merge($second, ...$rest);
  return filter_keys(
    $first,
    $key ==> !C\contains_key($union, $key),
  );
}

/**
 * Returns a new dict containing all except the first `$n` entries of the
 * given KeyedTraversable.
 *
 * To take only the first `$n` entries, see `Dict\take()`.
 *
 * Time complexity: O(n), where n is the size of `$traversable`
 * Space complexity: O(n), where n is the size of `$traversable`
 */
function drop<Tk as arraykey, Tv>(
  KeyedTraversable<Tk, Tv> $traversable,
  int $n,
)[]: dict<Tk, Tv> {
  invariant($n >= 0, 'Expected non-negative N, got %d.', $n);
  $result = dict[];
  $ii = -1;
  foreach ($traversable as $key => $value) {
    $ii++;
    if ($ii < $n) {
      continue;
    }
    $result[$key] = $value;
  }
  return $result;
}

/**
 * Returns a new dict containing only the values for which the given predicate
 * returns `true`. The default predicate is casting the value to boolean.
 *
 * - To remove null values in a typechecker-visible way, see `Dict\filter_nulls()`.
 * - To use an async predicate, see `Dict\filter_async()`.
 *
 * Time complexity: O(n * p), where p is the complexity of `$value_predicate`
 * (which is O(1) if not provided explicitly)
 * Space complexity: O(n)
 */
function filter<Tk as arraykey, Tv>(
  KeyedTraversable<Tk, Tv> $traversable,
  ?(function(Tv)[_]: bool) $value_predicate = null,
)[ctx $value_predicate]: dict<Tk, Tv> {
  $value_predicate ??= \HH\Lib\_Private\boolval<>;
  $dict = dict[];
  foreach ($traversable as $key => $value) {
    if ($value_predicate($value)) {
      $dict[$key] = $value;
    }
  }
  return $dict;
}

/**
 * Just like filter, but your predicate can include the key as well as
 * the value.
 *
 * To use an async predicate, see `Dict\filter_with_key_async()`.
 *
 * Time complexity: O(n * p), where p is the complexity of `$value_predicate`
 * (which is O(1) if not provided explicitly)
 * Space complexity: O(n)
 */
function filter_with_key<Tk as arraykey, Tv>(
  KeyedTraversable<Tk, Tv> $traversable,
  (function(Tk, Tv)[_]: bool) $predicate,
)[ctx $predicate]: dict<Tk, Tv> {
  $dict = dict[];
  foreach ($traversable as $key => $value) {
    if ($predicate($key, $value)) {
      $dict[$key] = $value;
    }
  }
  return $dict;
}

/**
 * Returns a new dict containing only the keys for which the given predicate
 * returns `true`. The default predicate is casting the key to boolean.
 *
 * Time complexity: O(n * p), where p is the complexity of `$value_predicate`
 * (which is O(1) if not provided explicitly)
 * Space complexity: O(n)
 */
function filter_keys<Tk as arraykey, Tv>(
  KeyedTraversable<Tk, Tv> $traversable,
  ?(function(Tk)[_]: bool) $key_predicate = null,
)[ctx $key_predicate]: dict<Tk, Tv> {
  $key_predicate ??= \HH\Lib\_Private\boolval<>;
  $dict = dict[];
  foreach ($traversable as $key => $value) {
    if ($key_predicate($key)) {
      $dict[$key] = $value;
    }
  }
  return $dict;
}

/**
 * Given a KeyedTraversable with nullable values, returns a new dict with
 * those entries removed.
 *
 * Time complexity: O(n)
 * Space complexity: O(n)
 */
function filter_nulls<Tk as arraykey, Tv>(
  KeyedTraversable<Tk, ?Tv> $traversable,
)[]: dict<Tk, Tv> {
  $result = dict[];
  foreach ($traversable as $key => $value) {
    if ($value !== null) {
      $result[$key] = $value;
    }
  }
  return $result;
}

/**
 * Returns a new dict containing only the keys found in both the input container
 * and the given Traversable. The dict will have the same ordering as the
 * `$keys` Traversable.
 *
 * Time complexity: O(k), where k is the size of `$keys`.
 * Space complexity: O(k), where k is the size of `$keys`.
 */
function select_keys<Tk as arraykey, Tv>(
  KeyedContainer<Tk, Tv> $container,
  Traversable<Tk> $keys,
)[]: dict<Tk, Tv> {
  $result = dict[];
  if (!C\is_empty($container)) {
    foreach ($keys as $key) {
      if (C\contains_key($container, $key)) {
        $result[$key] = $container[$key];
      }
    }
  }
  return $result;
}

/**
 * Returns a new dict containing the first `$n` entries of the given
 * KeyedTraversable.
 *
 * To drop the first `$n` entries, see `Dict\drop()`.
 *
 * Time complexity: O(n), where n is `$n`
 * Space complexity: O(n), where n is `$n`
 */
function take<Tk as arraykey, Tv>(
  KeyedTraversable<Tk, Tv> $traversable,
  int $n,
)[]: dict<Tk, Tv> {
  if ($n === 0) {
    return dict[];
  }
  invariant($n > 0, 'Expected non-negative length, got %d.', $n);
  $result = dict[];
  $ii = 0;
  foreach ($traversable as $key => $value) {
    $result[$key] = $value;
    $ii++;
    if ($ii === $n) {
      break;
    }
  }
  return $result;
}

/**
 * Returns a new dict in which each value appears exactly once. In case of
 * duplicate values, later keys will overwrite the previous ones.
 *
 * For non-arraykey values, see `Dict\unique_by()`.
 *
 * Time complexity: O(n)
 * Space complexity: O(n)
 */
function unique<Tk as arraykey, Tv as arraykey>(
  KeyedTraversable<Tk, Tv> $traversable,
)[]: dict<Tk, Tv> {
  return flip(flip($traversable));
}

/**
 * Returns a new dict in which each value appears exactly once, where the
 * value's uniqueness is determined by transforming it to a scalar via the
 * given function. In case of duplicate scalar values, later keys will overwrite
 * the previous ones.
 *
 * For arraykey values, see `Dict\unique()`.
 *
 * Time complexity: O(n * s), where s is the complexity of `$scalar_func`
 * Space complexity: O(n)
 */
function unique_by<Tk as arraykey, Tv, Ts as arraykey>(
  KeyedContainer<Tk, Tv> $container,
  (function(Tv)[_]: Ts) $scalar_func,
)[ctx $scalar_func]: dict<Tk, Tv> {
  // We first convert the container to dict[scalar_key => original_key] to
  // remove duplicates, then back to dict[original_key => original_value].
  return $container
    |> pull_with_key(
      $$,
      ($k, $_) ==> $k,
      ($_, $v) ==> $scalar_func($v),
    )
    |> pull(
      $$,
      $orig_key ==> $container[$orig_key],
      $x ==> $x,
    );
}
