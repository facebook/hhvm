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
final class TSimpleJSONProtocolMapContext extends TSimpleJSONProtocolContext {
  private bool $first = true;
  private bool $colon = true;

  <<__Override>>
  public function writeStart()[write_props]: int {
    $this->trans->write('{');
    return 1;
  }

  <<__Override>>
  public function writeSeparator()[write_props]: int {
    if ($this->first) {
      $this->first = false;
      return 0;
    }

    if ($this->colon) {
      $this->trans->write(':');
    } else {
      $this->trans->write(',');
    }

    $this->colon = !$this->colon;
    return 1;
  }

  <<__Override>>
  public function writeEnd()[write_props]: int {
    $this->trans->write('}');
    return 1;
  }

  <<__Override>>
  public function readStart()[write_props]: int {
    $result = $this->skipWhitespace();
    $c = $this->trans->readAll(1);
    $result++;
    if ($c !== '{') {
      throw new TProtocolException(
        'TSimpleJSONProtocol: Expected "{", encountered 0x'.PHP\bin2hex($c),
      );
    }
    return $result;
  }

  <<__Override>>
  public function readSeparator()[write_props]: int {
    if ($this->first) {
      $this->first = false;
      return 0;
    }

    $result = $this->skipWhitespace();
    $c = $this->trans->readAll(1);
    $result++;
    $target = $this->colon ? ':' : ',';

    if ($c !== $target) {
      throw new TProtocolException(
        'TSimpleJSONProtocol: Expected "'.
        $target.
        '", encountered 0x'.
        PHP\bin2hex($c),
      );
    }

    $this->colon = !$this->colon;
    return $result;
  }

  <<__Override>>
  public function readContextOver()[write_props]: bool {
    $pos = $this->skipWhitespace(false);
    $c = $this->trans->peek(1, $pos);
    if (!$this->first && $c !== ',' && $c !== '}') {
      throw new TProtocolException(
        'TSimpleJSONProtocol: Expected "," or "}", encountered 0x'.
        PHP\bin2hex($c),
      );
    }

    return ($c === '}');
  }

  <<__Override>>
  public function readEnd()[write_props]: int {
    $result = $this->skipWhitespace();
    $c = $this->trans->readAll(1);
    $result++;
    if ($c !== '}') {
      throw new TProtocolException(
        'TSimpleJSONProtocol: Expected "}", encountered 0x'.PHP\bin2hex($c),
      );
    }
    return $result;
  }

  <<__Override>>
  public function escapeNum()[]: bool {
    return $this->colon;
  }
}
