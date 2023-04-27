<?hh
/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @package thrift
 */

/**
 * Event handler for thrift processors
 */
class TProcessorEventHandler {

  // Called at the start of processing a handler method
  public function getHandlerContext(string $fn_name): mixed {
    return null;
  }

  // Called before the handler method's argument are read
  public function preRead(
    mixed $handler_context,
    string $fn_name,
    mixed $args,
  ): void {}

  // Called after the handler method's argument are read
  public function postRead(
    mixed $handler_context,
    string $fn_name,
    mixed $args,
  ): void {}

  // Called right before the handler is executed.
  // Exceptions thrown here are caught by handlerException and handlerError.
  public function preExec(
    mixed $handler_context,
    string $service_name,
    string $fn_name,
    mixed $args,
  ): void {}

  // Called right after the handler is executed.
  // Exceptions thrown here are caught by handlerException and handlerError.
  public function postExec(
    mixed $handler_context,
    string $fn_name,
    mixed $result,
  ): void {}

  // Called before the handler method's $results are written
  public function preWrite(
    mixed $handler_context,
    string $fn_name,
    mixed $result,
  ): void {}

  // Called after the handler method's $results are written
  public function postWrite(
    mixed $handler_context,
    string $fn_name,
    mixed $result,
  ): void {}

  // Called if (and only if) the handler threw an expected $exception.
  public function handlerException(
    mixed $handler_context,
    string $fn_name,
    Exception $exception,
  ): void {}

  /**
   * Called if (and only if) the handler threw an unexpected $exception.
   *
   * Note that this method is NOT called if the handler threw an
   * exception that is declared in the thrift service specification
   */
  public function handlerError(
    mixed $handler_context,
    string $fn_name,
    Exception $exception,
  ): void {}
}
