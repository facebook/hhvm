<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 *
 */

namespace HH\Lib\Async;

/** An async poll/select equivalent for traversables without a related key.
 *
 * ===== WARNING ===== WARNING ===== WARNING ===== WARNING ===== WARNING =====
 *
 * See detailed warning at top of `BasePoll`
 *
 * ===== WARNING ===== WARNING ===== WARNING ===== WARNING ===== WARNING =====
 */

final class Poll<Tv> extends BasePoll<mixed, Tv> implements AsyncIterator<Tv> {
  /** Create a Poll from the specified list of awaitables.
   *
   * See `KeyedPoll` if you have a `KeyedTraversable` and want to preserve
   * keys.
   */
  public static function from(Traversable<Awaitable<Tv>> $awaitables): this {
    return self::fromImpl(new Vector($awaitables));
  }

  /** Add an additional awaitable to the poll. */
  public function add(Awaitable<Tv> $awaitable): void {
    $this->addImpl(null, $awaitable);
  }

  /** Add multiple additional awaitables to the poll.
   *
   * See `KeyedPoll` if you have a `KeyedTraversable` and want to preserve keys.
   */
  public function addMulti(Traversable<Awaitable<Tv>> $awaitables): void {
    $this->addMultiImpl(new Vector($awaitables));
  }
}
