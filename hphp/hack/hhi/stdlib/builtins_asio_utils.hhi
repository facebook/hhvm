<?hh // decl
/* Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
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

function mm<Tk, Tv, Tr>(
  KeyedTraversable<Tk, Tv> $inputs,
  (function (Tv): Awaitable<Tr>) $callable,
): Awaitable<Map<Tk, Tr>> {}

function mmk<Tk, Tv, Tr>(
  KeyedTraversable<Tk, Tv> $inputs,
  (function (Tk, Tv): Awaitable<Tr>) $callable,
): Awaitable<Map<Tk, Tr>> {}

function mf<Tk, Tv>(
  KeyedTraversable<Tk, Tv> $inputs,
  (function (Tv): Awaitable<bool>) $callable,
): Awaitable<Map<Tk, Tv>> {}

function mfk<Tk, Tv>(
  KeyedTraversable<Tk, Tv> $inputs,
  (function (Tk, Tv): Awaitable<bool>) $callable,
): Awaitable<Map<Tk, Tv>> {}

function mw<Tk, Tv>(
  KeyedTraversable<Tk, Awaitable<Tv>> $awaitables,
): Awaitable<Map<Tk, ResultOrExceptionWrapper<Tv>>> {}

function mmw<Tk, Tv, Tr>(
  KeyedTraversable<Tk, Tv> $inputs,
  (function (Tv): Awaitable<Tr>) $callable,
): Awaitable<Map<Tk, ResultOrExceptionWrapper<Tr>>> {}

function mmkw<Tk, Tv, Tr>(
  KeyedTraversable<Tk, Tv> $inputs,
  (function (Tk, Tv): Awaitable<Tr>) $callable,
): Awaitable<Map<Tk, ResultOrExceptionWrapper<Tr>>> {}

function mfw<Tk,T>(
  KeyedTraversable<Tk, T> $inputs,
  (function (T): Awaitable<bool>) $callable,
): Awaitable<Map<Tk, ResultOrExceptionWrapper<T>>> {}

function mfkw<Tk, T>(
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

function vf<Tk, T>(
  KeyedTraversable<Tk, T> $inputs,
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

function vfw<Tk,T>(
  KeyedTraversable<Tk, T> $inputs,
  (function (T): Awaitable<bool>) $callable,
): Awaitable<Vector<ResultOrExceptionWrapper<T>>> {}

function vfkw<Tk, T>(
  KeyedTraversable<Tk, T> $inputs,
  (function (Tk, T): Awaitable<bool>) $callable,
): Awaitable<Vector<ResultOrExceptionWrapper<T>>> {}

function m<Tk, Tv>(
  KeyedTraversable<Tk, Awaitable<Tv>> $awaitables,
): Awaitable<Map<Tk, Tv>> {}

function v<Tv>(
  Traversable<Awaitable<Tv>> $awaitables,
): Awaitable<Vector<Tv>> {}

} // namespace HH\Asio
