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
interface IThriftRemoteConn extends TTransportStatus {
  public function open()[zoned_shallow]: void;
  public function close()[zoned_shallow]: void;
  public function flush()[zoned_shallow]: void;
  public function isOpen()[]: bool;

  public function read(int $len)[zoned_shallow]: string;
  public function readAll(int $len)[zoned_shallow]: string;
  public function write(string $buf)[zoned_shallow]: void;
  public function setRecvTimeout(int $timeout)[write_props]: void;

  public function getHost()[leak_safe]: string;
  public function getPort()[leak_safe]: int;
}
