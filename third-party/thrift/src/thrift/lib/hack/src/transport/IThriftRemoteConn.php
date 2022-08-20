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
 * @package thrift.transport
 */

interface IThriftRemoteConn extends TTransportStatus {
  public function open(): void;
  public function close(): void;
  public function flush(): void;
  public function isOpen(): bool;

  public function read(int $len): string;
  public function readAll(int $len): string;
  public function write(string $buf): void;

  public function getRecvTimeout(): int;
  public function setRecvTimeout(int $timeout): void;

  public function getHost(): string;
  public function getPort(): int;
}
