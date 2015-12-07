<?hh // decl /* -*- mode: php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

/**
 * This file provides type information for some of PHP's predefined classes
 *
 * YOU SHOULD NEVER INCLUDE THIS FILE ANYWHERE!!!
 */

/**
 * Async generators are similar to
 * [PHP Generators](http://php.net/manual/en/language.generators.overview.php),
 * except that we are combining async with generators.
 *
 * An async generator is just like a normal generator with the addition of
 * allowing `await` statements in it because getting to the next yielded value
 * involves getting and awaiting on an `Awaitable`.
 *
 * WHILE THIS CLASS EXPOSES 3 METHODS, 99.9% OF THE TIME YOU WILL NOT USE THIS
 * CLASS DIRECTLY. INSTEAD, YOU WILL USE `await as` IN THE CODE USING YOUR
 * ASYNC GENERATOR. PLEASE READ THE GUIDE REFERENCED IN THIS API DOCUMENTATION
 * FOR MORE INFORMATION. However, we document these methods for completeness in
 * case you have a use case for them.
 *
 * There are three type parameters associated with an AsyncGenerator:
 * - `Tk`: The type of key returned by the generator
 * - `Tv`: The type of value returned by the generator
 * - `Ts`: The type that will be passed on a call to `send()`
 *
 * @guide /hack/async/generators
 */
final class AsyncGenerator<Tk, +Tv, -Ts>
    implements AsyncKeyedIterator<Tk, Tv> {
  /**
   * Return the `Awaitable` associated with the next key/value tuple in the
   * async generator, or `null`.
   *
   * You should always `await` the returned `Awaitable` to get the actual
   * key/value tuple.
   *
   * If `null` is returned, that means you have reached the end of iteration.
   *
   * You cannot call `next()` without having the value returned from a previous
   * call to `next()`, `send()`, `raise()`, having first `await`ed.
   *
   * @return - The `Awaitable` that produced the next key/value tuple in the
   *           generator. What is returned is a tuple or `null`.
   */
  public function next(): Awaitable<?(Tk, Tv)> {}
  /**
   * Send a value to the async generator and resumes execution of the generator.
   *
   * You should always `await` the returned `Awaitable` to get the actual
   * key/value tuple.
   *
   * If `null` is returned, that means you have reached the end of iteration.
   *
   * You cannot call `send()` without having the value returned from a previous
   * call to `send()`, `next()`, `raise()`, having first `await`ed.
   *
   * If you pass `null` to `send()`, that is equivalent to calling `next()`,
   * but you still need an initial `next()` call before calling `send(null)`.
   *
   * @param $v - The value to send to the async generator.
   *
   * @return - The `Awaitable` that produced the yielded key/value tuple in
   *           the generator. What is returned is a tuple or `null`.
   */
  public function send(?Ts $v): Awaitable<?(Tk, Tv)> {}
  /**
   * Raise an exception to the async generator.
   *
   * You should always `await` the returned `Awaitable` to get the actual
   * key/value tuple.
   *
   * If `null` is returned, that means you have reached the end of iteration.
   *
   * You cannot call `raise()` without having the value returned from a previous
   * call to `raise()`, `next()`, `send()`, having first `await`ed.
   *
   * @param $e - The exception to raise on the async generator.
   *
   * @return - The `Awaitable` that produced the yielded key/value tuple after
   *           the exception is processed. What is returned is a tuple or
   *           `null`.
   */
  public function raise(Exception $e): Awaitable<?(Tk, Tv)> {}
}

final class Generator<Tk, +Tv, -Ts> implements KeyedIterator<Tk, Tv> {
  public function getOrigFuncName(): string {}
  public function current(): Tv {}
  public function key(): Tk {}
  public function valid(): bool {}
  public function next(): void {}
  public function send(?Ts $v): void {}
  public function raise(Exception $e): void {}
  public function rewind(): void {}
}

abstract class WaitHandle<+T> implements Awaitable<T> {
  public function getWaitHandle(): this {}
  public function import(): void {}
  public function join(): T {}
  public function isFinished(): bool {}
  public function isSucceeded(): bool {}
  public function isFailed(): bool {}
  public function getID(): int {}
  public function getName(): string {}
  public function result() : T {}
  public static function setOnIOWaitEnterCallback(?(function(): void) $callback) {}
  public static function setOnIOWaitExitCallback(?(function(): void) $callback) {}
  public static function setOnJoinCallback(?(function(WaitableWaitHandle<mixed>): void) $callback) {}
}

final class StaticWaitHandle<+T> extends WaitHandle<T> {
}

abstract class WaitableWaitHandle<+T> extends WaitHandle<T> {
  public function getContextIdx(): int {}
  public function getCreator(): /*AsyncFunction*/WaitHandle<mixed> {}
  public function getParents(): array<WaitableWaitHandle<mixed>> {}
}

abstract class ResumableWaitHandle<+T> extends WaitableWaitHandle<T> {
  public static function setOnCreateCallback(?(function(AsyncFunctionWaitHandle<mixed>, WaitableWaitHandle<mixed>): void) $callback) {}
  public static function setOnAwaitCallback(?(function(AsyncFunctionWaitHandle<mixed>, WaitableWaitHandle<mixed>): void) $callback) {}
  public static function setOnSuccessCallback(?(function(AsyncFunctionWaitHandle<mixed>, mixed): void) $callback) {}
  public static function setOnFailCallback(?(function(AsyncFunctionWaitHandle<mixed>, Exception): void) $callback) {}
}

final class AsyncFunctionWaitHandle<+T> extends ResumableWaitHandle<T> {
}

final class AsyncGeneratorWaitHandle<Tk, +Tv>
  extends ResumableWaitHandle<?(Tk, Tv)> {
}

final class AwaitAllWaitHandle extends WaitableWaitHandle<void> {
  public static function fromArray<T>(
    array<WaitHandle<T>> $deps
  ): WaitHandle<void>;
  public static function fromMap<Tk, Tv>(
    ConstMap<Tk, WaitHandle<Tv>> $deps
  ): WaitHandle<void>;
  public static function fromVector<T>(
    ConstVector<WaitHandle<T>> $deps
  ): WaitHandle<void>;
  public static function setOnCreateCallback(
    ?(function(AwaitAllWaitHandle, Vector<mixed>): void) $callback
  ): void {}
}

final class GenArrayWaitHandle extends WaitableWaitHandle<array> {
  // This is technically overloaded to allow an array of nullable
  public static function create(array $dependencies): WaitHandle<array> {}
  public static function setOnCreateCallback(?(function(GenArrayWaitHandle, array): void) $callback) {}
}

final class GenMapWaitHandle<Tk, Tv> extends WaitableWaitHandle<Map<Tk, Tv>> {
  public static function create(Map<Tk, WaitHandle<Tv>> $dependencies): WaitHandle<Map<Tk, Tv>> {}
  public static function setOnCreateCallback(?(function(GenMapWaitHandle<Tk, Tv>, Map<mixed, mixed>): void) $callback) {}
}

final class GenVectorWaitHandle<T> extends WaitableWaitHandle<Vector<T>> {
  public static function create(Vector<WaitHandle<T>> $dependencies): WaitHandle<Vector<T>> {}
  public static function setOnCreateCallback(?(function(GenVectorWaitHandle<T>, Vector<mixed>): void) $callback) {}
}

final class ConditionWaitHandle<T> extends WaitableWaitHandle<T> {
  public static function create(WaitHandle<void> $child): ConditionWaitHandle<T> {}
  public static function setOnCreateCallback(?(function(ConditionWaitHandle<T>, WaitableWaitHandle<void>): void) $callback) {}
  public function succeed(T $result): void {}
  public function fail(Exception $exception): void {}
}

final class RescheduleWaitHandle extends WaitableWaitHandle<void> {
  const int QUEUE_DEFAULT = 0;
  const int QUEUE_NO_PENDING_IO = 1;
  public static function create(int $queue, int $priority): RescheduleWaitHandle {}
}

final class SleepWaitHandle extends WaitableWaitHandle<void> {
  public static function create(int $usecs): SleepWaitHandle {}
  public static function setOnCreateCallback(?(function(SleepWaitHandle): void) $callback) {}
  public static function setOnSuccessCallback(?(function(SleepWaitHandle): void) $callback) {}
}

final class ExternalThreadEventWaitHandle<+T> extends WaitableWaitHandle<T> {
  public static function setOnCreateCallback(?(function(ExternalThreadEventWaitHandle<mixed>): void) $callback) {}
  public static function setOnSuccessCallback(?(function(ExternalThreadEventWaitHandle<mixed>, mixed): void) $callback) {}
  public static function setOnFailCallback(?(function(ExternalThreadEventWaitHandle<mixed>, Exception): void) $callback) {}
}

/*
 * stdClass is not really final. However, because stdClass has no
 * properties of its own and is the result of casting an array to an
 * object, it is exempt from 'property must exist' checks and so should not
 * be getting extended.
 */
final class stdClass {}

class __PHP_Incomplete_Class {}
