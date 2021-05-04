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

use namespace HH\Lib\{C, Dict};

/**
 * Returns a new dict with each value `await`ed in parallel.
 *
 * Time complexity: O(n * a), where a is the complexity of the synchronous
 * portions of each Awaitable
 * Space complexity: O(n)
 *
 * The IO operations for each Awaitable will happen in parallel.
 */
async function from_async<Tk as arraykey, Tv>(
  KeyedTraversable<Tk, Awaitable<Tv>> $awaitables,
)[]: Awaitable<dict<Tk, Tv>> {
  $dict = cast_clear_legacy_array_mark($awaitables);

  /* HH_FIXME[4390] Magic Function */
  await AwaitAllWaitHandle::fromDict($dict);
  foreach ($dict as $key => $value) {
    /* HH_FIXME[4390] Magic Function */
    $dict[$key] = \HH\Asio\result($value);
  }
  /* HH_FIXME[4110] Reuse the existing dict to reduce peak memory. */
  return $dict;
}

/**
 * Returns a new dict where each value is the result of calling the given
 * async function on the corresponding key.
 *
 * For non-async functions, see `Dict\from_keys()`.
 *
 * Time complexity: O(n * f), where f is the complexity of `$async_func`
 * Space complexity: O(n)
 */
async function from_keys_async<Tk as arraykey, Tv>(
  Traversable<Tk> $keys,
  (function(Tk)[_]: Awaitable<Tv>) $async_func,
)[ctx $async_func]: Awaitable<dict<Tk, Tv>> {
  $awaitables = dict[];
  foreach ($keys as $key) {
    $awaitables[$key] ??= $async_func($key);
  }
  /* HH_FIXME[4135] Unset local variable to reduce peak memory. */
  unset($keys);

  /* HH_FIXME[4390] Magic Function */
  await AwaitAllWaitHandle::fromDict($awaitables);
  foreach ($awaitables as $key => $value) {
    /* HH_FIXME[4390] Magic Function */
    $awaitables[$key] = \HH\Asio\result($value);
  }
  /* HH_FIXME[4110] Reuse the existing dict to reduce peak memory. */
  return $awaitables;
}

/**
 * Returns a new dict containing only the values for which the given async
 * predicate returns `true`.
 *
 * For non-async predicates, see `Dict\filter()`.
 *
 * Time complexity: O(n * p), where p is the complexity of the synchronous
 * portions of `$value_predicate`
 * Space complexity: O(n)
 *
 * The IO operations for each of the calls to `$value_predicate` will happen
 * in parallel.
 */
async function filter_async<Tk as arraykey, Tv>(
  KeyedContainer<Tk, Tv> $traversable,
  (function(Tv)[_]: Awaitable<bool>) $value_predicate,
)[ctx $value_predicate]: Awaitable<dict<Tk, Tv>> {
  $tests = await map_async($traversable, $value_predicate);
  $result = dict[];
  foreach ($traversable as $key => $value) {
    if ($tests[$key]) {
      $result[$key] = $value;
    }
  }
  return $result;
}

/**
 * Like `filter_async`, but lets you utilize the keys of your dict too.
 *
 * For non-async filters with key, see `Dict\filter_with_key()`.
 *
 * Time complexity: O(n * p), where p is the complexity of `$value_predicate`
 * Space complexity: O(n)
 */
async function filter_with_key_async<Tk as arraykey, Tv>(
  KeyedContainer<Tk, Tv> $traversable,
  (function(Tk, Tv)[_]: Awaitable<bool>) $predicate,
)[ctx $predicate]: Awaitable<dict<Tk, Tv>> {
  $tests = await map_with_key_async($traversable, $predicate);
  $result = dict[];
  foreach ($tests as $k => $v) {
    if ($v) {
      $result[$k] = $traversable[$k];
    }
  }
  return $result;
}

/**
 * Returns a new dict where each value is the result of calling the given
 * async function on the original value.
 *
 * For non-async functions, see `Dict\map()`.
 *
 * Time complexity: O(n * f), where f is the complexity of the synchronous
 * portions of `$async_func`
 * Space complexity: O(n)
 *
 * The IO operations for each of calls to `$async_func` will happen in
 * parallel.
 */
async function map_async<Tk as arraykey, Tv1, Tv2>(
  KeyedTraversable<Tk, Tv1> $traversable,
  (function(Tv1)[_]: Awaitable<Tv2>) $value_func,
)[ctx $value_func]: Awaitable<dict<Tk, Tv2>> {
  $dict = cast_clear_legacy_array_mark($traversable);
  foreach ($dict as $key => $value) {
    $dict[$key] = $value_func($value);
  }

  /* HH_FIXME[4110] Okay to pass in Awaitable */
  /* HH_FIXME[4390] Magic Function */
  await AwaitAllWaitHandle::fromDict($dict);
  foreach ($dict as $key => $value) {
    /* HH_FIXME[4110] Reuse the existing dict to reduce peak memory. */
    /* HH_FIXME[4390] Magic Function */
    $dict[$key] = \HH\Asio\result($value);
  }
  /* HH_FIXME[4110] Reuse the existing dict to reduce peak memory. */
  return $dict;
}

/**
 * Returns a new dict where each value is the result of calling the given
 * async function on the original key and value.
 *
 * For non-async functions, see `Dict\map()`.
 *
 * Time complexity: O(n * a), where a is the complexity of each Awaitable
 * Space complexity: O(n)
 */
async function map_with_key_async<Tk as arraykey, Tv1, Tv2>(
  KeyedTraversable<Tk, Tv1> $traversable,
  (function(Tk, Tv1)[_]: Awaitable<Tv2>) $async_func,
)[ctx $async_func]: Awaitable<dict<Tk, Tv2>> {
  $awaitables = map_with_key($traversable, $async_func);
  /* HH_FIXME[4135] Unset local variable to reduce peak memory. */
  unset($traversable);

  /* HH_FIXME[4390] Magic Function */
  await AwaitAllWaitHandle::fromDict($awaitables);
  foreach ($awaitables as $index => $value) {
    /* HH_FIXME[4390] Magic Function */
    $awaitables[$index] = \HH\Asio\result($value);
  }
  /* HH_FIXME[4110] Reuse the existing dict to reduce peak memory. */
  return $awaitables;
}
