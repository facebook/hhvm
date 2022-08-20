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
 * @package thrift.protocol.simplejson
 */

class TSimpleJSONProtocolContext {
  final public function __construct(
    protected TTransport $trans,
    protected IThriftBufferedTransport $bufTrans,
  ) {}

  public function writeStart(): int {
    return 0;
  }

  public function writeSeparator(): int {
    return 0;
  }

  public function writeEnd(): int {
    return 0;
  }

  public function readStart(): void {
    // Do nothing
  }

  public function readSeparator(): void {
    // Do nothing
  }

  public function readContextOver(): bool {
    return true;
  }

  public function readEnd(): void {
    // Do nothing
  }

  public function escapeNum(): bool {
    return false;
  }

  protected function skipWhitespace(bool $skip = true): int {
    $count = 0;
    $reading = true;
    while ($reading) {
      $byte = $this->bufTrans->peek(1, $count);
      switch ($byte) {
        case ' ':
        case "\t":
        case "\n":
        case "\r":
          $count++;
          break;
        default:
          $reading = false;
          break;
      }
    }
    if ($skip) {
      $this->trans->readAll($count);
    }
    return $count;
  }
}
