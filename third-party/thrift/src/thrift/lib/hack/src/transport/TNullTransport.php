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

/**
 * Transport that only accepts writes and ignores them.
 * This is useful for measuring the serialized size of structures.
 *
 * @package thrift.transport
 */
<<Oncalls('thrift')>> // @oss-disable
final class TNullTransport
  extends TWritePropsTransport
  implements IThriftBufferedTransport {

  <<__Override>>
  public function isOpen()[]: bool {
    return true;
  }

  <<__Override>>
  public function open()[]: void {}

  <<__Override>>
  public function close()[]: void {}

  <<__Override>>
  public function read(int $len)[]: string {
    throw new TTransportException("Can't read from TNullTransport.");
  }

  public function peek(int $len, int $start = 0)[]: string {
    throw new TTransportException("Can't peek through TNullTransport.");
  }

  public function putBack(string $data)[]: void {}

  <<__Override>>
  public function write(string $buf)[]: void {}

}
