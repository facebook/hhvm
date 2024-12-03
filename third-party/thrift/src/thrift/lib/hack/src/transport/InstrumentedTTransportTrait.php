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
 * TTransport subclasses that implement InstrumentedTTransport and use
 * InstrumentedTTransportTrait have a simple set of counters available.
 */
<<Oncalls('thrift')>> // @oss-disable
trait InstrumentedTTransportTrait {
  private int $bytesWritten = 0;
  private int $bytesRead = 0;

  public function getBytesWritten()[]: int {
    return $this->bytesWritten;
  }

  public function getBytesRead()[]: int {
    return $this->bytesRead;
  }

  public function resetBytesWritten()[write_props]: void {
    $this->bytesWritten = 0;
  }

  public function resetBytesRead()[write_props]: void {
    $this->bytesRead = 0;
  }

  protected function onWrite(int $bytes_written)[write_props]: void {
    $this->bytesWritten += $bytes_written;
  }

  protected function onRead(int $bytes_read)[write_props]: void {
    $this->bytesRead += $bytes_read;
  }
}
