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
 * Binary Protocol Factory
 */
<<Oncalls('thrift')>> // @oss-disable
final class TBinaryProtocolFactory implements TProtocolFactory {
  protected bool $strictRead = false;
  protected bool $strictWrite = true;

  public function __construct(
    bool $strict_read = false,
    bool $strict_write = true,
  )[] {
    $this->strictRead = $strict_read;
    $this->strictWrite = $strict_write;
  }

  public function getProtocol(TTransport $trans)[read_globals]: TProtocol {
    return new TBinaryProtocolAccelerated(
      $trans is IThriftBufferedTransport
        ? $trans
        : new TBufferedTransport<TTransport>($trans),
      $this->strictRead,
      $this->strictWrite,
    );
  }
}
