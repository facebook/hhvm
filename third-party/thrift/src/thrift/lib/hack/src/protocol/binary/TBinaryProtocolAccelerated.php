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
 * Accelerated binary protocol: used in conjunction with the thrift_protocol
 * extension for faster deserialization
 */
<<Oncalls('thrift')>> // @oss-disable
class TBinaryProtocolAccelerated extends TBinaryProtocolBase {

  public function __construct(
    IThriftBufferedTransport $trans,
    bool $strict_read = false,
    bool $strict_write = true,
  )[read_globals] {
    parent::__construct($trans, $strict_read, $strict_write);
  }

  public function isStrictRead()[]: bool {
    return $this->strictRead;
  }

  public function isStrictWrite()[]: bool {
    return $this->strictWrite;
  }
}
