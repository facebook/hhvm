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
function asio_get_current_context_idx();
function asio_get_running_in_context($ctx_idx);
function asio_get_running();

namespace HH\Asio {
  function join<T>(Awaitable<T> $awaitable): T;
  function result<T>(Awaitable<T> $awaitable): T;
  function has_finished<T>(Awaitable<T> $awaitable): bool;
  function cancel<T>(Awaitable<T> $awaitable, \Exception $exception): bool;
}
