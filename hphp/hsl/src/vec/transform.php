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

use namespace HH\Lib\Math;

/**
 * Returns a vec containing the original vec split into chunks of the given
 * size. If the original vec doesn't divide evenly, the final chunk will be
 * smaller.
 *
 * Time complexity: O(n)
 * Space complexity: O(n)
 */
<<__NoAutoLikes>>
function chunk<Tv>(
  Traversable<Tv> $traversable,
  int $size,
)[]: vec<vec<Tv>> {
  invariant($size > 0, 'Expected positive chunk size, got %d.', $size);
  $result = vec[];
  $ii = 0;
  $chunk_number = -1;
  foreach ($traversable as $value) {
    if ($ii % $size === 0) {
      $result[] = vec[];
      $chunk_number++;
    }

    $result[$chunk_number][] = $value;
    $ii++;
  }
  return $result;
}

/**
 * Returns a new vec of size `$size` where all the values are `$value`.
 *
 * If you need a range of items not repeats, use `Vec\range(0, $n - 1)`.
 *
 * Time complexity: O(n)
 * Space complexity: O(n)
 */
<<__NoAutoLikes>>
function fill<Tv>(
  int $size,
  Tv $value,
)[]: vec<Tv> {
  invariant($size >= 0, 'Expected non-negative fill size, got %d.', $size);
  $result = vec[];
  for ($i = 0; $i < $size; $i++) {
    $result[] = $value;
  }
  return $result;
}

/**
 * Returns a new vec formed by joining the Traversable elements of the given
 * Traversable.
 *
 * For a fixed number of Traversables, see `Vec\concat()`.
 *
 * Time complexity: O(n), where n is the combined size of all the
 * `$traversables`
 * Space complexity: O(n), where n is the combined size of all the
 * `$traversables`
 */
<<__NoAutoLikes>>
function flatten<Tv>(
  Traversable<Container<Tv>> $traversables,
)[]: vec<Tv> {
  $result = vec[];
  foreach ($traversables as $traversable) {
    foreach ($traversable as $value) {
      $result[] = $value;
    }
  }
  return $result;
}

/**
 * Returns a new vec where each value is the result of calling the given
 * function on the original value.
 *
 * For async functions, see `Vec\map_async()`.
 *
 * Time complexity: O(n)
 * Space complexity: O(n)
 */
function map<Tv1, Tv2>(
  Traversable<Tv1> $traversable,
  (function(Tv1)[_]: Tv2) $value_func,
)[ctx $value_func]: vec<Tv2> {
  $result = vec[];
  foreach ($traversable as $value) {
    $result[] = $value_func($value);
  }
  return $result;
}

/**
 * Returns a new vec where each value is the result of calling the given
 * function on the original key and value.
 *
 * Time complexity: O(n * f), where f is the complexity of `$value_func`
 * Space complexity: O(n)
 */
function map_with_key<Tk, Tv1, Tv2>(
  KeyedTraversable<Tk, Tv1> $traversable,
  (function(Tk, Tv1)[_]: Tv2) $value_func,
)[ctx $value_func]: vec<Tv2> {
  $result = vec[];
  foreach ($traversable as $key => $value) {
    $result[] = $value_func($key, $value);
  }
  return $result;
}
