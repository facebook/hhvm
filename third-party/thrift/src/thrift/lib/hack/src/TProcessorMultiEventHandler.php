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
 */

// @oss-enable: use namespace FlibSL\{C, Math, Str, Vec};

<<Oncalls('thrift')>> // @oss-disable
final class TProcessorMultiEventHandler extends TProcessorEventHandler {
  private Map<string, TProcessorEventHandler> $handlers;

  public function __construct() {
    $this->handlers = Map {};
  }

  public function addHandler(
    string $key,
    TProcessorEventHandler $handler,
  )[write_props]: this {
    $this->handlers[$key] = $handler;
    return $this;
  }

  public function getHandler(string $key)[]: TProcessorEventHandler {
    return $this->handlers[$key];
  }

  public function removeHandler(
    string $key,
  )[write_props]: TProcessorEventHandler {
    $handler = $this->getHandler($key);
    $this->handlers->remove($key);
    return $handler;
  }

  // Called at the start of processing a handler method
  <<__Override>>
  public function getHandlerContext(string $fn_name): mixed {
    $context = Map {};
    foreach ($this->handlers as $key => $handler) {
      $context[$key] = $handler->getHandlerContext($fn_name);
    }

    return $context;
  }

  // Called before the handler method's argument are read
  <<__Override>>
  public function preRead(
    mixed $handler_context,
    string $fn_name,
    mixed $args,
  ): void {
    invariant($handler_context is Map<_, _>, 'Context is not a Map');
    foreach ($this->handlers as $key => $handler) {
      $handler->preRead(
        $handler_context->at(HH\FIXME\UNSAFE_CAST<
          string,
          HH_FIXME\UNKNOWN_TYPE_FOR_CAST,
        >(
          $key,
          'FIXME[4110]: Type error revealed by type-safe instanceof feature. See https://fburl.com/instanceof',
        )),
        $fn_name,
        $args,
      );
    }
  }

  // Called after the handler method's argument are read
  <<__Override>>
  public function postRead(
    mixed $handler_context,
    string $fn_name,
    mixed $args,
  ): void {
    invariant($handler_context is Map<_, _>, 'Context is not a Map');
    foreach ($this->handlers as $key => $handler) {
      $handler->postRead(
        $handler_context->at(HH\FIXME\UNSAFE_CAST<
          string,
          HH_FIXME\UNKNOWN_TYPE_FOR_CAST,
        >(
          $key,
          'FIXME[4110]: Type error revealed by type-safe instanceof feature. See https://fburl.com/instanceof',
        )),
        $fn_name,
        $args,
      );
    }
  }

  // Called right before the handler is executed.
  // Exceptions thrown here are caught by handlerException and handlerError.
  <<__Override>>
  public function preExec(
    mixed $handler_context,
    string $service_name,
    string $fn_name,
    mixed $args,
  ): void {
    invariant($handler_context is Map<_, _>, 'Context is not a Map');
    foreach (
      PHPism_FIXME::coerceKeyedTraversableOrObject($this->handlers) as
        $key => $handler
    ) {
      $handler->preExec(
        $handler_context->at(HH\FIXME\UNSAFE_CAST<
          string,
          HH_FIXME\UNKNOWN_TYPE_FOR_CAST,
        >(
          $key,
          'FIXME[4110] Exposed by adding return type to PHPism_FIXME::coerce(Keyed)?TraversableOrObject',
        )),
        $service_name,
        $fn_name,
        $args,
      );
    }
  }

  // Called right after the handler is executed.
  // Exceptions thrown here are caught by handlerException and handlerError.
  <<__Override>>
  public function postExec(
    mixed $handler_context,
    string $fn_name,
    mixed $result,
  ): void {
    invariant($handler_context is Map<_, _>, 'Context is not a Map');
    foreach ($this->handlers as $key => $handler) {
      $handler->postExec(
        $handler_context->at(HH\FIXME\UNSAFE_CAST<
          string,
          HH_FIXME\UNKNOWN_TYPE_FOR_CAST,
        >(
          $key,
          'FIXME[4110]: Type error revealed by type-safe instanceof feature. See https://fburl.com/instanceof',
        )),
        $fn_name,
        $result,
      );
    }
  }

  // Called before the handler method's $results are written
  <<__Override>>
  public function preWrite(
    mixed $handler_context,
    string $fn_name,
    mixed $result,
  ): void {
    invariant($handler_context is Map<_, _>, 'Context is not a Map');
    foreach ($this->handlers as $key => $handler) {
      $handler->preWrite(
        $handler_context->at(HH\FIXME\UNSAFE_CAST<
          string,
          HH_FIXME\UNKNOWN_TYPE_FOR_CAST,
        >(
          $key,
          'FIXME[4110]: Type error revealed by type-safe instanceof feature. See https://fburl.com/instanceof',
        )),
        $fn_name,
        $result,
      );
    }
  }

  // Called after the handler method's $results are written
  <<__Override>>
  public function postWrite(
    mixed $handler_context,
    string $fn_name,
    mixed $result,
  ): void {
    invariant($handler_context is Map<_, _>, 'Context is not a Map');
    foreach ($this->handlers as $key => $handler) {
      $handler->postWrite(
        $handler_context->at(HH\FIXME\UNSAFE_CAST<
          string,
          HH_FIXME\UNKNOWN_TYPE_FOR_CAST,
        >(
          $key,
          'FIXME[4110]: Type error revealed by type-safe instanceof feature. See https://fburl.com/instanceof',
        )),
        $fn_name,
        $result,
      );
    }
  }

  // Called if (and only if) the handler threw an expected $exception.
  <<__Override>>
  public function handlerException(
    mixed $handler_context,
    string $fn_name,
    Exception $ex,
  ): void {
    invariant($handler_context is Map<_, _>, 'Context is not a Map');
    foreach ($this->handlers as $key => $handler) {
      $handler->handlerException(
        $handler_context->at(HH\FIXME\UNSAFE_CAST<
          string,
          HH_FIXME\UNKNOWN_TYPE_FOR_CAST,
        >(
          $key,
          'FIXME[4110]: Type error revealed by type-safe instanceof feature. See https://fburl.com/instanceof',
        )),
        $fn_name,
        $ex,
      );
    }
  }

  /**
   * Called if (and only if) the handler threw an unexpected $exception.
   *
   * Note that this method is NOT called if the handler threw an
   * exception that is declared in the thrift service specification
   */
  <<__Override>>
  public function handlerError(
    mixed $handler_context,
    string $fn_name,
    Exception $ex,
  ): void {
    invariant($handler_context is Map<_, _>, 'Context is not a Map');
    foreach ($this->handlers as $key => $handler) {
      $handler->handlerError(
        $handler_context->at(HH\FIXME\UNSAFE_CAST<
          string,
          HH_FIXME\UNKNOWN_TYPE_FOR_CAST,
        >(
          $key,
          'FIXME[4110]: Type error revealed by type-safe instanceof feature. See https://fburl.com/instanceof',
        )),
        $fn_name,
        $ex,
      );
    }
  }
}
