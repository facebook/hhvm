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

use namespace HH\Lib\C;

/** Run an operation with a limit on number of ongoing asynchronous jobs.
 *
 * All operations must have the same input type (`Tin`) and output type (`Tout`),
 * and be processed by the same function; `Tin` may be a callable invoked by the
 * funtion for maximum flexibility, however this pattern is best avoided in favor
 * of creating semaphores with a more narrow process.
 *
 * Use `genWaitFor()` to retrieve a `Tout` from a `Tin`.
 */
final class Semaphore<Tin, Tout> {

  private static int $uniqueIDCounter = 0;
  private dict<int, Condition<null>> $blocking = dict[];
  private Awaitable<void> $activeGen;
  private int $runningCount = 0;
  private int $recentOpenCount = 0;

  /** Create a semaphore.
   *
   * The concurrent limit is per instance; for example, if there are two ongoing requests
   * executing the same code, there will be up to concurrentLimit in each request, meaning
   * up to 2 * concurrentLimit in total.
   */
  public function __construct(
    private int $concurrentLimit,
    private (function(Tin): Awaitable<Tout>) $f,
  ) {
    invariant($concurrentLimit > 0, "Concurrent limit must be greater than 0.");
    $this->activeGen = async {};
  }

  /** Produce a `Tout` from a `Tin`, respecting the concurrency limit. */
  public async function waitForAsync(Tin $value): Awaitable<Tout> {
    $gen = async {
      if (
        $this->runningCount + $this->recentOpenCount >= $this->concurrentLimit
      ) {
        $unique_id = self::$uniqueIDCounter;
        self::$uniqueIDCounter++;
        $condition = new Condition();
        $this->blocking[$unique_id] = $condition;
        await $condition->waitForNotificationAsync($this->activeGen);
        invariant(
          $this->recentOpenCount > 0,
          'Expecting at least one recentOpenCount.',
        );
        $this->recentOpenCount--;
      }
      invariant(
        $this->runningCount < $this->concurrentLimit,
        'Expecting open run slot',
      );
      $f = $this->f;
      $this->runningCount++;
      try {
        return await $f($value);
      } finally {
        $this->runningCount--;
        $next_blocked_id = C\first_key($this->blocking);
        if ($next_blocked_id !== null) {
          $next_blocked = $this->blocking[$next_blocked_id];
          unset($this->blocking[$next_blocked_id]);
          $this->recentOpenCount++;
          $next_blocked->succeed(null);
        }
      }
    };
    $this->activeGen = AwaitAllWaitHandle::fromVec(
      vec[$gen, $this->activeGen],
    );
    return await $gen;
  }
}
