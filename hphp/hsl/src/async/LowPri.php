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

/**
 * LowPri is a wrapper around ASIO machinery that allows running an async lambda
 * in a low-priority context, while waiting for the result in a non-low
 * (e.g. current) context.
 *
 * When the run() method is called, the async lambda begins executing in a
 * low-priority context. This means it will only execute when there is no other
 * higher-priority work to be done (e.g., during IO wait phases).
 *
 * LowPri objects can be stored and later prioritized when their results are
 * needed. This can be useful in cases where it would be helpful to start some
 * work early, but explicitly not block work on the critical path.
 */
final class LowPri<T> {
  // PBWH backing this object, bridges priority between current context and
  // a low priority context to run the action in.
  private ?PriorityBridgeWaitHandle<T> $wh = null;

  // Track whether the low-pri action has been prioritized before $wh is
  // available
  private bool $isPrioritized = false;

  <<__AsioLowPri>>
  private static async function lowpri(
    (function()[_]: Awaitable<T>) $action,
  )[ctx $action]: Awaitable<T> {
    return await $action();
  }

  /**
   * Run the provided `$action` in a low-priority context. The action will begin
   * executing immediately but at a lower priority, meaning it will only run
   * when there is no higher-priority work to be done. Raises an
   * InvariantException if called more than once on the same object.
   *
   * @param $action The async lambda to run in a low-priority context.
   * @return Awaitable<T> An awaitable that resolves with the result of the
   * action.
   */
  public async function run(
    (function()[_]: Awaitable<T>) $action,
  )[ctx $action, write_props]: Awaitable<T> {
    invariant($this->wh is null, 'Unable to run LowPri twice');

    $child = self::lowpri($action);
    $this->wh = PriorityBridgeWaitHandle::create($child);
    if ($this->isPrioritized) {
      $this->wh->prioritize();
    }
    return await $this->wh;
  }

  /**
   * Bring the low-priority action into the object's context by lifting its
   * priority.
   *
   * This method increases the priority of the running action to match the
   * object's context, ensuring it will execute with normal priority from this
   * point forward. If called before run(), the action will start with normal
   * priority when run() is called.
   */
  public function prioritize()[write_props]: void {
    $this->isPrioritized = true;
    $this->wh?->prioritize();
  }
}
