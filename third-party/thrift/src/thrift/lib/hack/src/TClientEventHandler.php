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

/**
 * Event handler for thrift clients
 */
class TClientEventHandler {
  public function setClient(IThriftClient $client): void {}
  // Called before the request is sent to the server
  public function preSend(
    string $fn_name,
    mixed $args,
    int $sequence_id,
  ): void {}

  // Called after the request is sent to the server
  public function postSend(
    string $fn_name,
    mixed $args,
    int $sequence_id,
  ): void {}

  // Called if there is an exception writing the data
  public function sendError(
    string $fn_name,
    mixed $args,
    int $sequence_id,
    Exception $ex,
  ): void {}

  // Called before the response is read from the server
  // Exceptions thrown here are caught by clientException and clientError.
  public function preRecv(string $fn_name, ?int $ex_sequence_id): void {}

  // Called after the response is read from the server
  public function postRecv(
    string $fn_name,
    ?int $ex_sequence_id,
    mixed $result,
  ): void {}

  // Called if (and only if) the client threw an expected $exception.
  public function recvException(
    string $fn_name,
    ?int $ex_sequence_id,
    TException $exception,
  ): void {}

  // Called if (and only if) the client threw an unexpected $exception.
  public function recvError(
    string $fn_name,
    ?int $ex_sequence_id,
    Exception $exception,
  ): void {}
}
