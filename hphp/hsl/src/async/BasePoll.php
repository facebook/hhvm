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

/**
 * Asynchronous equivalent of mechanisms such as epoll(), poll() and select().
 *
 * Read the warnings here first, then see the `Poll` and `KeyedPoll`
 * instantiable subclasses.
 *
 * Transforms a set of Awaitables to an asynchronous iterator that produces
 * results of these Awaitables as soon as they are ready. The order of results
 * is not guaranteed in any way. New Awaitables can be added to the Poll
 * while it is being iterated.
 *
 * This mechanism has two primary use cases:
 *
 * 1. Speculatively issuing non-CPU-intensive requests to different backends
 *    with very high processing latency, waiting for the first satisfying
 *    result and ignoring all remaining requests.
 *
 *    Example: cross-DC memcache requests
 *
 * 2. Processing relatively small number of high level results in the order
 *    of completion and flushing the output to the user.
 *
 *    Example: pagelets, multiple GraphQL queries, streamable GraphQL queries
 *
 * ===== WARNING ===== WARNING ===== WARNING ===== WARNING ===== WARNING =====
 *
 * This is a very heavy-weight mechanism with non-trivial CPU cost. NEVER use
 * this in the following situations:
 *
 * 1. Waiting for the first available result and ignoring the rest of work,
 *    unless the processing latency is extremely high (10ms or more) and
 *    the CPU cost of ignored work is negligible. Note: the ignored work
 *    will still be computed and will delay your processing anyway if it's
 *    CPU costly.
 *
 * 2. Reordering huge amount of intermediary results. This is currently known
 *    to be CPU-intensive.
 *
 * ===== WARNING ===== WARNING ===== WARNING ===== WARNING ===== WARNING =====
 */

<<__ConsistentConstruct>>
abstract class BasePoll<Tk, Tv> {
  final public static function create(): this {
    return new static();
  }

  final protected static function fromImpl(
    KeyedTraversable<Tk, Awaitable<Tv>> $awaitables,
  ): this {
    $poll = new static();
    $poll->addMultiImpl($awaitables);
    return $poll;
  }

  private ?ConditionNode<(Tk, Tv)> $lastAdded;
  private ?ConditionNode<(Tk, Tv)> $lastNotified;
  private ?ConditionNode<(Tk, Tv)> $lastAwaited;
  private Awaitable<void> $notifiers;

  private function __construct() {
    $head = new ConditionNode();
    $this->lastAdded = $head;
    $this->lastNotified = $head;
    $this->lastAwaited = $head;
    $this->notifiers = async {
    };
  }

  final protected function addImpl(Tk $key, Awaitable<Tv> $awaitable): void {
    invariant(
      $this->lastAdded !== null,
      'Unable to add item, iteration already finished',
    );

    // Create condition node representing pending event.
    $this->lastAdded = $this->lastAdded->addNext();

    // Make sure the next pending condition is notified upon completion.
    $awaitable = $this->waitForThenNotify($key, $awaitable);

    // Keep track of all pending events.
    $this->notifiers = AwaitAllWaitHandle::fromVec(
      vec[$awaitable, $this->notifiers],
    );
  }

  final protected function addMultiImpl(
    KeyedTraversable<Tk, Awaitable<Tv>> $awaitables,
  ): void {
    invariant(
      $this->lastAdded !== null,
      'Unable to add item, iteration already finished',
    );
    $last_added = $this->lastAdded;

    // Initialize new list of notifiers.
    $notifiers = vec[$this->notifiers];

    foreach ($awaitables as $key => $awaitable) {
      // Create condition node representing pending event.
      $last_added = $last_added->addNext();

      // Make sure the next pending condition is notified upon completion.
      $awaitable = $this->waitForThenNotify($key, $awaitable);
      $notifiers[] = $awaitable;
    }

    // Keep track of all pending events.
    $this->lastAdded = $last_added;
    $this->notifiers = AwaitAllWaitHandle::fromVec($notifiers);
  }

  <<__ProvenanceSkipFrame>>
  private async function waitForThenNotify(
    Tk $key,
    Awaitable<Tv> $awaitable,
  ): Awaitable<void> {
    try {
      $result = await $awaitable;
      $this->lastNotified = ($this->lastNotified as nonnull)->getNext() as
        nonnull;
      $this->lastNotified->succeed(tuple($key, $result));
    } catch (\Exception $exception) {
      $this->lastNotified = ($this->lastNotified as nonnull)->getNext() as
        nonnull;
      $this->lastNotified->fail($exception);
    }
  }

  <<__ProvenanceSkipFrame>>
  final public async function next(): Awaitable<?(Tk, Tv)> {
    invariant(
      $this->lastAwaited !== null,
      'Unable to iterate, iteration already finished',
    );

    $this->lastAwaited = $this->lastAwaited->getNext();
    if ($this->lastAwaited === null) {
      // End of iteration, no pending events to await.
      $this->lastAdded = null;
      $this->lastNotified = null;
      return null;
    }

    return await $this->lastAwaited->waitForNotificationAsync($this->notifiers);
  }

  final public function hasNext(): bool {
    invariant(
      $this->lastAwaited !== null,
      'Unable to iterate, iteration already finished',
    );
    return $this->lastAwaited->getNext() !== null;
  }
}
