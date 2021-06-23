<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the hphp/hsl/ subdirectory of this source tree.
 *
 */

namespace HH\Lib\Async;

/** A keyed variant of `Poll`.
 *
 * See `Poll` if you do not need to preserve keys.
 *
 * Keys are retrieved with:
 *
 * ```
 * foreach ($keyed_poll await as $k => $v) {
 * ```
 *
 * ===== WARNING ===== WARNING ===== WARNING ===== WARNING ===== WARNING =====
 *
 * See detailed warning for `BasePoll`
 *
 * ===== WARNING ===== WARNING ===== WARNING ===== WARNING ===== WARNING =====
 */
final class KeyedPoll<Tk, Tv>
  extends BasePoll<Tk, Tv>
  implements AsyncKeyedIterator<Tk, Tv> {

  /** Create a `KeyedPoll` from the specified list of awaitables.
   *
   * See `Poll` if keys are unimportant.
   */
  public static function from(
    KeyedTraversable<Tk, Awaitable<Tv>> $awaitables,
  ): this {
    return self::fromImpl($awaitables);
  }

  /** Add a single awaitable to the poll.
   *
   * The key is retrieved with `foreach ($poll await as $k => $v) {}`
   */
  public function add(Tk $key, Awaitable<Tv> $awaitable): void {
    $this->addImpl($key, $awaitable);
  }

  /** Add multiple keys and awaitables to the poll */
  public function addMulti(
    KeyedTraversable<Tk, Awaitable<Tv>> $awaitables,
  ): void {
    $this->addMultiImpl($awaitables);
  }
}
