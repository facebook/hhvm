<?hh
/* Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

namespace HH\Asio {

interface ResultOrExceptionWrapper<T> {
  public function isSucceeded(): bool;
  public function isFailed(): bool;
  public function getResult(): T;
  public function getException(): \Exception;
}

final class WrappedException<Te as \Exception,Tr>
  implements ResultOrExceptionWrapper<Tr> {
  public function __construct(private Te $exception) {}

  public function isSucceeded(): bool {}
  public function isFailed(): bool {}

  public function getResult(): Tr {}
  public function getException(): Te {}
}

final class WrappedResult<T> implements ResultOrExceptionWrapper<T> {
  public function __construct(private T $result) {}

  public function isSucceeded(): bool {}
  public function isFailed(): bool {}

  public function getResult(): T {}

  public function getException(): \Exception {}
}

function wrap<Tv>(
  Awaitable<Tv> $awaitable,
): Awaitable<ResultOrExceptionWrapper<Tv>> {}

function later(): Awaitable<void> {}

function usleep(
  int $usecs,
): Awaitable<void> {}

function mm<Tk as arraykey, Tv, Tr>(
  KeyedTraversable<Tk, Tv> $inputs,
  (function (Tv): Awaitable<Tr>) $callable,
): Awaitable<Map<Tk, Tr>> {}

function mmk<Tk as arraykey, Tv, Tr>(
  KeyedTraversable<Tk, Tv> $inputs,
  (function (Tk, Tv): Awaitable<Tr>) $callable,
): Awaitable<Map<Tk, Tr>> {}

function mf<Tk as arraykey, Tv>(
  KeyedTraversable<Tk, Tv> $inputs,
  (function (Tv): Awaitable<bool>) $callable,
): Awaitable<Map<Tk, Tv>> {}

function mfk<Tk as arraykey, Tv>(
  KeyedTraversable<Tk, Tv> $inputs,
  (function (Tk, Tv): Awaitable<bool>) $callable,
): Awaitable<Map<Tk, Tv>> {}

function mw<Tk as arraykey, Tv>(
  KeyedTraversable<Tk, Awaitable<Tv>> $awaitables,
): Awaitable<Map<Tk, ResultOrExceptionWrapper<Tv>>> {}

function mmw<Tk as arraykey, Tv, Tr>(
  KeyedTraversable<Tk, Tv> $inputs,
  (function (Tv): Awaitable<Tr>) $callable,
): Awaitable<Map<Tk, ResultOrExceptionWrapper<Tr>>> {}

function mmkw<Tk as arraykey, Tv, Tr>(
  KeyedTraversable<Tk, Tv> $inputs,
  (function (Tk, Tv): Awaitable<Tr>) $callable,
): Awaitable<Map<Tk, ResultOrExceptionWrapper<Tr>>> {}

function mfw<Tk as arraykey,T>(
  KeyedTraversable<Tk, T> $inputs,
  (function (T): Awaitable<bool>) $callable,
): Awaitable<Map<Tk, ResultOrExceptionWrapper<T>>> {}

function mfkw<Tk as arraykey, T>(
  KeyedTraversable<Tk, T> $inputs,
  (function (Tk, T): Awaitable<bool>) $callable,
): Awaitable<Map<Tk, ResultOrExceptionWrapper<T>>> {}

function vm<Tv, Tr>(
  Traversable<Tv> $inputs,
  (function (Tv): Awaitable<Tr>) $callable,
): Awaitable<Vector<Tr>> {}

function vmk<Tk, Tv, Tr>(
  KeyedTraversable<Tk, Tv> $inputs,
  (function (Tk, Tv): Awaitable<Tr>) $callable,
): Awaitable<Vector<Tr>>;

function vf<T>(
  KeyedTraversable<mixed, T> $inputs,
  (function (T): Awaitable<bool>) $callable,
): Awaitable<Vector<T>> {}

function vfk<Tk, T>(
  KeyedTraversable<Tk, T> $inputs,
  (function (Tk, T): Awaitable<bool>) $callable,
): Awaitable<Vector<T>> {}

function vw<Tv>(
  Traversable<Awaitable<Tv>> $awaitables,
): Awaitable<Vector<ResultOrExceptionWrapper<Tv>>> {}

function vmw<Tv, Tr>(
  Traversable<Tv> $inputs,
  (function (Tv): Awaitable<Tr>) $callable,
): Awaitable<Vector<ResultOrExceptionWrapper<Tr>>> {}

function vmkw<Tk, Tv, Tr>(
  KeyedTraversable<Tk, Tv> $inputs,
  (function (Tk, Tv): Awaitable<Tr>) $callable,
): Awaitable<Vector<ResultOrExceptionWrapper<Tr>>> {}

function vfw<T>(
  KeyedTraversable<mixed, T> $inputs,
  (function (T): Awaitable<bool>) $callable,
): Awaitable<Vector<ResultOrExceptionWrapper<T>>> {}

function vfkw<Tk, T>(
  KeyedTraversable<Tk, T> $inputs,
  (function (Tk, T): Awaitable<bool>) $callable,
): Awaitable<Vector<ResultOrExceptionWrapper<T>>> {}

function m<Tk as arraykey, Tv>(
  KeyedTraversable<Tk, Awaitable<Tv>> $awaitables,
): Awaitable<Map<Tk, Tv>> {}

function v<Tv>(
  Traversable<Awaitable<Tv>> $awaitables,
): Awaitable<Vector<Tv>> {}

} // namespace HH\Asio
