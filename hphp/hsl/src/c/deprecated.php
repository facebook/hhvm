<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the hphp/hsl/ subdirectory of this source tree.
 *
 */

namespace HH\Lib\C;

/**
 * Returns the first element of the result of the given Awaitable, or null if
 * the Traversable is empty.
 *
 * For non-Awaitable Traversables, see `C\first`.
 *
 * Time complexity: O(1)
 * Space complexity: O(1)
 */
<<__Deprecated('use C\\first(await #A)')>>
async function first_async<T>(
  Awaitable<Traversable<T>> $awaitable,
): Awaitable<?T> {
  return first(await $awaitable);
}

/**
 * Returns the first element of the result of the given Awaitable, or throws if
 * the Traversable is empty.
 *
 * For non-Awaitable Traversables, see `C\firstx`.
 *
 * Time complexity: O(1)
 * Space complexity: O(1)
 */
<<__Deprecated('use C\\firstx(await #A)')>>
async function firstx_async<T>(
  Awaitable<Traversable<T>> $awaitable,
): Awaitable<T> {
  return firstx(await $awaitable);
}
