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
class TSimpleJSONProtocolContext {

  final public function __construct(protected TMemoryBuffer $trans)[] {}

  public function writeStart()[write_props]: int {
    return 0;
  }

  public function writeSeparator()[write_props]: int {
    return 0;
  }

  public function writeEnd()[write_props]: int {
    return 0;
  }

  public function readStart()[write_props]: int {
    // Do nothing
    return 0;
  }

  public function readSeparator()[write_props]: int {
    // Do nothing
    return 0;
  }

  public function readContextOver()[write_props]: bool {
    return true;
  }

  public function readEnd()[write_props]: int {
    // Do nothing
    return 0;
  }

  public function escapeNum()[]: bool {
    return false;
  }

  protected function skipWhitespace(bool $skip = true)[write_props]: int {
    $count = 0;
    $reading = true;
    while ($reading) {
      $byte = $this->trans->peek(1, $count);
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
