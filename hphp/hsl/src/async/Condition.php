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
 * A wrapper around ConditionWaitHandle that allows notification events
 * to occur before the condition is awaited.
 */
class Condition<T> {
  private ?Awaitable<T> $condition = null;

  /**
   * Notify the condition variable of success and set the result.
   */
  final public function succeed(T $result): void {
    if ($this->condition === null) {
      $this->condition = async {
        return $result;
      };
    } else {
      invariant(
        $this->condition is ConditionWaitHandle<_>,
        'Unable to notify AsyncCondition twice',
      );
      /* HH_FIXME[4110]: Type error revealed by type-safe instanceof feature. See https://fburl.com/instanceof */
      $this->condition->succeed($result);
    }
  }

  /**
   * Notify the condition variable of failure and set the exception.
   */
  final public function fail(\Exception $exception): void {
    if ($this->condition === null) {
      $this->condition = async {
        throw $exception;
      };
    } else {
      invariant(
        $this->condition is ConditionWaitHandle<_>,
        'Unable to notify AsyncCondition twice',
      );
      $this->condition->fail($exception);
    }
  }

  /**
   * Asynchronously wait for the condition variable to be notified and
   * return the result or throw the exception received via notification.
   *
   * The caller must provide an Awaitable $notifiers (which must be a
   * WaitHandle) that must not finish before the notification is received.
   * This means $notifiers must represent work that is guaranteed to
   * eventually trigger the notification. As long as the notification is
   * issued only once, asynchronous execution unrelated to $notifiers is
   * allowed to trigger the notification.
   */
  <<__ProvenanceSkipFrame>>
  final public async function waitForNotificationAsync(
    Awaitable<void> $notifiers,
  ): Awaitable<T> {
    if ($this->condition === null) {
      $this->condition = ConditionWaitHandle::create($notifiers);
    }
    return await $this->condition;
  }
}
