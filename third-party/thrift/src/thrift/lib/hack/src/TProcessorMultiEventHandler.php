<?hh
/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

final class TProcessorMultiEventHandler extends TProcessorEventHandler {
  private Map<string, TProcessorEventHandler> $handlers;

  public function __construct() {
    $this->handlers = Map {};
  }

  public function addHandler(
    string $key,
    TProcessorEventHandler $handler,
  ): this {
    $this->handlers[$key] = $handler;
    return $this;
  }

  public function getHandler(string $key): TProcessorEventHandler {
    return $this->handlers[$key];
  }

  public function removeHandler(string $key): TProcessorEventHandler {
    $handler = $this->getHandler($key);
    $this->handlers->remove($key);
    return $handler;
  }

  // Called at the start of processing a handler method
  public function getHandlerContext(string $fn_name): mixed {
    $context = Map {};
    foreach ($this->handlers as $key => $handler) {
      $context[$key] = $handler->getHandlerContext($fn_name);
    }

    return $context;
  }

  // Called before the handler method's argument are read
  public function preRead(
    mixed $handler_context,
    string $fn_name,
    mixed $args,
  ): void {
    invariant($handler_context instanceof Map, 'Context is not a Map');
    foreach ($this->handlers as $key => $handler) {
      $handler->preRead($handler_context->at($key), $fn_name, $args);
    }
  }

  // Called after the handler method's argument are read
  public function postRead(
    mixed $handler_context,
    string $fn_name,
    mixed $args,
  ): void {
    invariant($handler_context instanceof Map, 'Context is not a Map');
    foreach ($this->handlers as $key => $handler) {
      $handler->postRead($handler_context->at($key), $fn_name, $args);
    }
  }

  // Called right before the handler is executed.
  // Exceptions thrown here are caught by handlerException and handlerError.
  public function preExec(
    mixed $handler_context,
    string $service_name,
    string $fn_name,
    mixed $args,
  ): void {
    invariant($handler_context instanceof Map, 'Context is not a Map');
    foreach ($this->handlers as $key => $handler) {
      $handler->preExec(
        $handler_context->at($key),
        $service_name,
        $fn_name,
        $args
      );
    }
  }

  // Called right after the handler is executed.
  // Exceptions thrown here are caught by handlerException and handlerError.
  public function postExec(
    mixed $handler_context,
    string $fn_name,
    mixed $result,
  ): void {
    invariant($handler_context instanceof Map, 'Context is not a Map');
    foreach ($this->handlers as $key => $handler) {
      $handler->postExec($handler_context->at($key), $fn_name, $result);
    }
  }

  // Called before the handler method's $results are written
  public function preWrite(
    mixed $handler_context,
    string $fn_name,
    mixed $result,
  ): void {
    invariant($handler_context instanceof Map, 'Context is not a Map');
    foreach ($this->handlers as $key => $handler) {
      $handler->preWrite($handler_context->at($key), $fn_name, $result);
    }
  }

  // Called after the handler method's $results are written
  public function postWrite(
    mixed $handler_context,
    string $fn_name,
    mixed $result,
  ): void {
    invariant($handler_context instanceof Map, 'Context is not a Map');
    foreach ($this->handlers as $key => $handler) {
      $handler->postWrite($handler_context->at($key), $fn_name, $result);
    }
  }

  // Called if (and only if) the handler threw an expected $exception.
  public function handlerException(
    mixed $handler_context,
    string $fn_name,
    Exception $ex,
  ): void {
    invariant($handler_context instanceof Map, 'Context is not a Map');
    foreach ($this->handlers as $key => $handler) {
      $handler->handlerException($handler_context->at($key), $fn_name, $ex);
    }
  }

  /**
   * Called if (and only if) the handler threw an unexpected $exception.
   *
   * Note that this method is NOT called if the handler threw an
   * exception that is declared in the thrift service specification
   */
  public function handlerError(
    mixed $handler_context,
    string $fn_name,
    Exception $ex,
  ): void {
    invariant($handler_context instanceof Map, 'Context is not a Map');
    foreach ($this->handlers as $key => $handler) {
      $handler->handlerError($handler_context->at($key), $fn_name, $ex);
    }
  }
}
