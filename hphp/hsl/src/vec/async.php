<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 *
 */

namespace HH\Lib\Vec;

/**
 * Returns a new vec with each value `await`ed in parallel.
 *
 * Time complexity: O(n * a), where a is the complexity of synchronous
 * portions of each Awaitable
 * Space complexity: O(n)
 *
 * The IO operations for each Awaitable will happen in parallel.
 */
async function from_async<Tv>(
  Traversable<Awaitable<Tv>> $awaitables,
)[]: Awaitable<vec<Tv>> {
  $vec = cast_clear_legacy_array_mark($awaitables);

  /* HH_FIXME[4387] Hide the magic from reactivity */
  /* HH_FIXME[4390] Magic Function */
  await AwaitAllWaitHandle::fromVec($vec);
  foreach ($vec as $index => $value) {
    /* HH_FIXME[4387] Hide the magic from reactivity */
    /* HH_FIXME[4390] Magic Function */
    $vec[$index] = \HH\Asio\result($value);
  }
  /* HH_FIXME[4110] Reuse the existing vec to reduce peak memory. */
  return $vec;
}

/**
 * Returns a new vec containing only the values for which the given async
 * predicate returns `true`.
 *
 * For non-async predicates, see `Vec\filter()`.
 *
 * Time complexity: O(n * p), where p is the complexity of synchronous portions
 * of `$value_predicate`
 * Space complexity: O(n)
 *
 * The IO operations for each of the calls to `$value_predicate` will happen
 * in parallel.
 */
async function filter_async<Tv>(
  Container<Tv> $container,
  (function(Tv)[_]: Awaitable<bool>) $value_predicate,
)[ctx $value_predicate]: Awaitable<vec<Tv>> {
  $tests = await map_async($container, $value_predicate);
  $result = vec[];
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
 * Returns a new vec where each value is the result of calling the given
 * async function on the original value.
 *
 * For non-async functions, see `Vec\map()`.
 *
 * Time complexity: O(n * f), where `f` is the complexity of the synchronous
 * portions of `$async_func`
 * Space complexity: O(n)
 *
 * The IO operations for each of calls to `$async_func` will happen in
 * parallel.
 */
async function map_async<Tv1, Tv2>(
  Traversable<Tv1> $traversable,
  (function(Tv1)[_]: Awaitable<Tv2>) $async_func,
)[ctx $async_func]: Awaitable<vec<Tv2>> {
  $vec = cast_clear_legacy_array_mark($traversable);
  foreach ($vec as $i => $value) {
    /* HH_FIXME[4401] need to make this safe to coeffects */
    $vec[$i] = $async_func($value);
  }

  /* HH_FIXME[4110] Okay to pass in Awaitable */
  /* HH_FIXME[4387] Hide the magic from reactivity */
  /* HH_FIXME[4390] Magic Function */
  await AwaitAllWaitHandle::fromVec($vec);
  foreach ($vec as $index => $value) {
    /* HH_FIXME[4110] Reuse the existing vec to reduce peak memory. */
    /* HH_FIXME[4387] Hide the magic from reactivity */
    /* HH_FIXME[4390] Magic Function */
    $vec[$index] = \HH\Asio\result($value);
  }
  /* HH_FIXME[4110] Reuse the existing vec to reduce peak memory. */
  return $vec;
}
