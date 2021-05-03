<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 *
 */

namespace HH\Lib\Keyset;

use namespace HH\Lib\Vec;

/**
 * Returns a new keyset containing the awaited result of the given Awaitables.
 *
 * Time complexity: O(n * a), where a is the complexity of the synchronous
 * portions of each Awaitable
 * Space complexity: O(n)
 *
 * The IO operations for each Awaitable will happen in parallel.
 */
async function from_async<Tv as arraykey>(
  Traversable<Awaitable<Tv>> $awaitables,
)[]: Awaitable<keyset<Tv>> {
  return keyset(await Vec\from_async($awaitables));
}

/**
 * Returns a new keyset containing only the values for which the given async
 * predicate returns `true`.
 *
 * For non-async predicates, see `Keyset\filter()`.
 *
 * Time complexity: O(n * p), where p is the complexity of the synchronous
 * portions of `$value_predicate`
 * Space complexity: O(n)
 *
 * The IO operations for each of the calls to `$value_predicate` will happen
 * in parallel.
 */
async function filter_async<Tv as arraykey>(
  Container<Tv> $container,
  (function(Tv)[_]: Awaitable<bool>) $value_predicate,
)[ctx $value_predicate]: Awaitable<keyset<Tv>> {
  $tests = await Vec\map_async($container, $value_predicate);
  $result = keyset[];
  $ii = 0;
  foreach ($container as $value) {
    if ($tests[$ii]) {
      $result[] = $value;
    }
    $ii++;
  }
  return $result;
}

/**
 * Returns a new keyset where the value is the result of calling the
 * given async function on the original values in the given traversable.
 *
 * Time complexity: O(n * f), where f is the complexity of the synchronous
 * portions of `$async_func`
 * Space complexity: O(n)
 *
 * The IO operations for each of calls to `$async_func` will happen in
 * parallel.
 */
async function map_async<Tv, Tk as arraykey>(
  Traversable<Tv> $traversable,
  (function(Tv)[_]: Awaitable<Tk>) $async_func,
)[ctx $async_func]: Awaitable<keyset<Tk>> {
  return keyset(await Vec\map_async($traversable, $async_func));
}

/**
 * Returns a 2-tuple containing keysets for which the given async
 * predicate returned `true` and `false`, respectively.
 *
 * For non-async predicates, see `Keyset\partition()`.
 *
 * Time complexity: O(n * p), where p is the complexity of synchronous portions
 * of `$value_predicate`
 * Space complexity: O(n)
 *
 * The IO operations for each of the calls to `$value_predicate` will happen
 * in parallel.
 */
async function partition_async<Tv as arraykey>(
  Container<Tv> $container,
  (function(Tv)[_]: Awaitable<bool>) $value_predicate,
)[ctx $value_predicate]: Awaitable<(keyset<Tv>, keyset<Tv>)> {
  $tests = await Vec\map_async($container, $value_predicate);
  $success = keyset[];
  $failure = keyset[];
  $ii = 0;
  foreach ($container as $value) {
    if ($tests[$ii]) {
      $success[] = $value;
    } else {
      $failure[] = $value;
    }
    $ii++;
  }
  return tuple($success, $failure);
}
