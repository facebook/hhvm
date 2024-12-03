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
final class TClientMultiEventHandler extends TClientEventHandler {
  private Map<string, TClientEventHandler> $handlers;

  public function __construct()[] {
    $this->handlers = Map {};
  }

  public function addHandler(
    string $key,
    TClientEventHandler $handler,
  )[write_props]: this {
    $this->handlers[$key] = $handler;
    return $this;
  }

  public function getHandler(string $key)[]: TClientEventHandler {
    return $this->handlers[$key];
  }

  public function removeHandler(string $key)[write_props]: TClientEventHandler {
    $handler = $this->getHandler($key);
    $this->handlers->remove($key);
    return $handler;
  }

  public function getHandlers()[]: ConstMap<string, TClientEventHandler> {
    return new ImmMap($this->handlers);
  }

  <<__Override>>
  public function preSend(
    string $fn_name,
    mixed $args,
    int $sequence_id,
    string $service_interface,
  ): void {
    foreach ($this->handlers as $handler) {
      $handler->preSend($fn_name, $args, $sequence_id, $service_interface);
    }
  }

  <<__Override>>
  public function postSend(
    string $fn_name,
    mixed $args,
    int $sequence_id,
  ): void {
    foreach ($this->handlers as $handler) {
      $handler->postSend($fn_name, $args, $sequence_id);
    }
  }

  <<__Override>>
  public function sendError(
    string $fn_name,
    mixed $args,
    int $sequence_id,
    Exception $ex,
  ): void {
    foreach ($this->handlers as $handler) {
      $handler->sendError($fn_name, $args, $sequence_id, $ex);
    }
  }

  <<__Override>>
  public function preRecv(string $fn_name, ?int $ex_sequence_id): void {
    foreach ($this->handlers as $handler) {
      $handler->preRecv($fn_name, $ex_sequence_id);
    }
  }

  <<__Override>>
  public function postRecv(
    string $fn_name,
    ?int $ex_sequence_id,
    mixed $result,
  ): void {
    foreach ($this->handlers as $handler) {
      $handler->postRecv($fn_name, $ex_sequence_id, $result);
    }
  }

  <<__Override>>
  public function recvException(
    string $fn_name,
    ?int $ex_sequence_id,
    TException $exception,
  ): void {
    foreach ($this->handlers as $handler) {
      $handler->recvException($fn_name, $ex_sequence_id, $exception);
    }
  }

  <<__Override>>
  public function recvError(
    string $fn_name,
    ?int $ex_sequence_id,
    Exception $exception,
  ): void {
    foreach ($this->handlers as $handler) {
      $handler->recvError($fn_name, $ex_sequence_id, $exception);
    }
  }
}
