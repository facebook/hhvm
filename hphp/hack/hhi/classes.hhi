<?hh /* -*- mode: php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

/**
 * This file provides type information for some of PHP's predefined classes
 *
 * YOU SHOULD NEVER INCLUDE THIS FILE ANYWHERE!!!
 */

namespace {

  <<__SupportDynamicType>>
  final class Generator<+Tk, +Tv, -Ts> implements KeyedIterator<Tk, Tv> {
    public function getOrigFuncName(): string {}
    public function current(): Tv {}
    public function key(): Tk {}
    public function valid(): bool {}
    public function next(): void {}
    public function send(?Ts $v): void {}
    public function raise(Exception $e): void {}
    public function rewind(): void {}
  }

  final class stdClass {}

} // namespace

namespace HH {

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
   * @guide /hack/asynchronous-operations/generators
   */
  final class AsyncGenerator<+Tk, +Tv, -Ts>
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
    public function raise(\Exception $e): Awaitable<?(Tk, Tv)> {}
  }

  <<
    __Sealed(
      AwaitAllWaitHandle::class,
      ConcurrentWaitHandle::class,
      ConditionWaitHandle::class,
      ExternalThreadEventWaitHandle::class,
      RescheduleWaitHandle::class,
      ResumableWaitHandle::class,
      SleepWaitHandle::class,
    ),
    __SupportDynamicType,
  >>
  abstract class WaitableWaitHandle<+T>
    extends Awaitable<T> {
  }
  <<__SupportDynamicType>>
  final class StaticWaitHandle<+T> extends Awaitable<T> {
  }

  <<__SupportDynamicType>>
  final class AsyncFunctionWaitHandle<+T>
    extends ResumableWaitHandle<T> {
  }

  <<__SupportDynamicType>>
  final class AsyncGeneratorWaitHandle<
    +Tk,
    +Tv,
  > extends ResumableWaitHandle<?(Tk, Tv)> {
  }

  /**
   * An `Awaitable` value represents a value that is fetched
   * asynchronously, such as a database access. `Awaitable` values are
   * usually returned by `async` functions.
   *
   * Use `await` to wait for a single `Awaitable` value. If you have
   * multiple `Awaitable`s and you want to wait for all of them
   * together, use `concurrent` or helper functions like
   * `Vec\map_async`.
   *
   * `Awaitable` is not multithreading. Hack is single threaded, so
   * `Awaitable` allows you to wait for multiple external results at
   * once, rather than sequentially.
   */
  <<
    __Sealed(StaticWaitHandle::class, WaitableWaitHandle::class),
    __SupportDynamicType,
  >>
  abstract class Awaitable<+T> {
    public static function setOnIOWaitEnterCallback(
      ?(function(): void) $callback,
    ): \HH\FIXME\MISSING_RETURN_TYPE {}
    public static function setOnIOWaitExitCallback(
      ?(function(?WaitableWaitHandle<mixed>): void) $callback,
    ): \HH\FIXME\MISSING_RETURN_TYPE {}
    public static function setOnJoinCallback(
      ?(function(WaitableWaitHandle<mixed>): void) $callback,
    ): \HH\FIXME\MISSING_RETURN_TYPE {}
  }

  <<
    __Sealed(AsyncFunctionWaitHandle::class, AsyncGeneratorWaitHandle::class),
    __SupportDynamicType,
  >>
  abstract class ResumableWaitHandle<+T>
    extends WaitableWaitHandle<T> {
    public static function setOnCreateCallback(
      ?(function(
        AsyncFunctionWaitHandle<mixed>,
        WaitableWaitHandle<mixed>,
      ): void) $callback,
    ): \HH\FIXME\MISSING_RETURN_TYPE {}
    public static function setOnAwaitCallback(
      ?(function(
        AsyncFunctionWaitHandle<mixed>,
        WaitableWaitHandle<mixed>,
      ): void) $callback,
    ): \HH\FIXME\MISSING_RETURN_TYPE {}
    public static function setOnSuccessCallback(
      ?(function(AsyncFunctionWaitHandle<mixed>, mixed): void) $callback,
    ): \HH\FIXME\MISSING_RETURN_TYPE {}
    public static function setOnFailCallback(
      ?(function(AsyncFunctionWaitHandle<mixed>, \Exception): void) $callback,
    ): \HH\FIXME\MISSING_RETURN_TYPE {}
  }

  <<__SupportDynamicType>>
  final class AwaitAllWaitHandle extends WaitableWaitHandle<void> {
    public static function fromDict(
      dict<arraykey, Awaitable<mixed>> $deps,
    )[]: Awaitable<void>;
    public static function fromVec(
      vec<Awaitable<mixed>> $deps,
    )[]: Awaitable<void>;
    public static function setOnCreateCallback(
      ?(function(AwaitAllWaitHandle, vec<WaitableWaitHandle<mixed>>): void) $callback,
    ): void {}
  }

  <<__SupportDynamicType>>
  final class ConcurrentWaitHandle extends WaitableWaitHandle<void> {
    public static function setOnCreateCallback(
      ?(function(ConcurrentWaitHandle, vec<WaitableWaitHandle<mixed>>): void) $callback,
    ): void {}
  }

  <<
    __SupportDynamicType,
  >> // TODO: determine whether it's safe to mark this as unconditionally supporting dynamic
  final class ConditionWaitHandle<T> extends WaitableWaitHandle<T> {
    public static function create(
      Awaitable<void> $child,
    ): ConditionWaitHandle<T> {}
    public static function setOnCreateCallback(
      ?(function(
        ConditionWaitHandle<T>,
        WaitableWaitHandle<void>,
      ): void) $callback,
    ): \HH\FIXME\MISSING_RETURN_TYPE {}
    public function succeed(T $result): void {}
    public function fail(\Exception $exception): void {}
  }

  <<__SupportDynamicType>>
  final class RescheduleWaitHandle extends WaitableWaitHandle<void> {
    const int QUEUE_DEFAULT;
    const int QUEUE_NO_PENDING_IO;
    public static function create(
      int $queue,
      int $priority,
    )[]: RescheduleWaitHandle {}
  }

  <<__SupportDynamicType>>
  final class SleepWaitHandle extends WaitableWaitHandle<void> {
    public static function create(int $usecs): SleepWaitHandle {}
    public static function setOnCreateCallback(
      ?(function(SleepWaitHandle): void) $callback,
    ): \HH\FIXME\MISSING_RETURN_TYPE {}
    public static function setOnSuccessCallback(
      ?(function(SleepWaitHandle): void) $callback,
    ): \HH\FIXME\MISSING_RETURN_TYPE {}
  }

  <<__SupportDynamicType>>
  final class ExternalThreadEventWaitHandle<+T>
    extends WaitableWaitHandle<T> {
    public static function setOnCreateCallback(
      ?(function(ExternalThreadEventWaitHandle<mixed>): void) $callback,
    ): \HH\FIXME\MISSING_RETURN_TYPE {}
    public static function setOnSuccessCallback(
      ?(function(ExternalThreadEventWaitHandle<mixed>, mixed): void) $callback,
    ): \HH\FIXME\MISSING_RETURN_TYPE {}
    public static function setOnFailCallback(
      ?(function(
        ExternalThreadEventWaitHandle<mixed>,
        \Exception,
      ): void) $callback,
    ): \HH\FIXME\MISSING_RETURN_TYPE {}
  }

  function is_class(mixed $arg)[]: bool;
} // namespace HH
