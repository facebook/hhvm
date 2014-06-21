<?hh // decl
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

class Exception {
  protected string $message;
  protected int $code;
  protected ?Exception $previous = null;
  protected string $file;
  protected int $line;
  protected array $trace;

  public function __construct (string $message = "", int $code = 0,
                               ?Exception $previous = null) {}
  public function getMessage(): string {}
  final public function getPrevious(): Exception {}
  public final function setPrevious(Exception $previous): void;
  public function getCode(): int {}
  final public function getFile(): string {}
  final public function getLine(): int {}
  final public function getTrace(): array {}
  final public function getTraceAsString(): string {}
  public function __toString(): string {}
  final private function __clone(): void {}

  public static function getTraceOptions() {}
  public static function setTraceOptions($opts) {}
}

class InvalidArgumentException extends Exception {
}

class RuntimeException extends Exception {
}

class OutOfBoundsException extends RuntimeException {
}

class AsyncGenerator<Tk, Tv, Ts> implements AsyncIterator<Tk, Tv> {
  public function next(): Awaitable<?(Tk, Tv)> {}
  public function send(Ts $v): Awaitable<?(Tk, Tv)> {}
  public function raise(Exception $e): Awaitable<?(Tk, Tv)> {}
}

class Continuation<Tv> implements KeyedIterator<int, Tv> {
  public function getOrigFuncName(): string {}
  public function current(): Tv {}
  public function key(): int {}
  public function valid(): bool {}
  public function next(): void {}
  public function send($v): void {}
  public function raise(Exception $e): void {}
  public function rewind(): void {}
  public function getLabel(): int {}
  public function update(int $label, Tv $value): void {}
  public function num_args(): int {}
  public function get_arg(int $index): mixed {}
}

class Generator<Tv> extends Continuation<Tv> {
}

abstract class WaitHandle<T> implements Awaitable<T> {
  public function getWaitHandle(): this {}
  public function import(): void {}
  public function join(): T {}
  public function isFinished(): bool {}
  public function isSucceeded(): bool {}
  public function isFailed(): bool {}
  public function getID(): int {}
  public function getName(): string {}
  public function getExceptionIfFailed(): ?Exception {}
  public static function setOnJoinCallback(?(function(WaitableWaitHandle<mixed>): void) $callback) {}
}

class StaticWaitHandle<T> extends WaitHandle<T> {
}

abstract class WaitableWaitHandle<T> extends WaitHandle<T> {
  public function getContextIdx(): int {}
  public function getCreator(): /*AsyncFunction*/WaitHandle<mixed> {}
  public function getParents(): array<BlockableWaitHandle<mixed>> {}
  public function getStackTrace(): array<WaitableWaitHandle<mixed>> {}
}

abstract class BlockableWaitHandle<T> extends WaitableWaitHandle<T> {
}

class AsyncFunctionWaitHandle<T> extends BlockableWaitHandle<T> {
  public static function setOnCreateCallback(?(function(AsyncFunctionWaitHandle<mixed>, WaitableWaitHandle<mixed>): void) $callback) {}
  public static function setOnAwaitCallback(?(function(AsyncFunctionWaitHandle<mixed>, WaitableWaitHandle<mixed>): void) $callback) {}
  public static function setOnSuccessCallback(?(function(AsyncFunctionWaitHandle<mixed>, mixed): void) $callback) {}
  public static function setOnFailCallback(?(function(AsyncFunctionWaitHandle<mixed>, Exception): void) $callback) {}
}

class GenArrayWaitHandle extends BlockableWaitHandle<array> {
  // This is technically overloaded to allow an array of nullable
  public static function create(array $dependencies): WaitHandle<array> {}
  public static function setOnCreateCallback(?(function(GenArrayWaitHandle, array): void) $callback) {}
}

class GenMapWaitHandle<Tk, Tv> extends BlockableWaitHandle<Map<Tk, Tv>> {
  public static function create(Map<Tk, WaitHandle<Tv>> $dependencies): WaitHandle<Map<Tk, Tv>> {}
  public static function setOnCreateCallback(?(function(GenMapWaitHandle<Tk, Tv>, Map<mixed, mixed>): void) $callback) {}
}

class GenVectorWaitHandle<T> extends BlockableWaitHandle<Vector<T>> {
  public static function create(Vector<WaitHandle<T>> $dependencies): WaitHandle<Vector<T>> {}
  public static function setOnCreateCallback(?(function(GenVectorWaitHandle<T>, Vector<mixed>): void) $callback) {}
}

class RescheduleWaitHandle extends WaitableWaitHandle<void> {
  const int QUEUE_DEFAULT = 0;
  const int QUEUE_NO_PENDING_IO = 1;
  public static function create(int $queue, int $priority): RescheduleWaitHandle {}
}

class SessionScopedWaitHandle<T> extends WaitableWaitHandle<T> {}

class SleepWaitHandle extends SessionScopedWaitHandle<void> {
  public static function create(int $usecs): SleepWaitHandle {}
}
