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

class TSimpleJSONProtocolMapContext extends TSimpleJSONProtocolContext {
  private bool $first = true;
  private bool $colon = true;

  public function writeStart(): int {
    $this->trans->write('{');
    return 1;
  }

  public function writeSeparator(): int {
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

  public function writeEnd(): int {
    $this->trans->write('}');
    return 1;
  }

  public function readStart(): void {
    $this->skipWhitespace();
    $c = $this->trans->readAll(1);
    if ($c !== '{') {
      throw new TProtocolException(
        'TSimpleJSONProtocol: Expected "{", encountered 0x'.bin2hex($c),
      );
    }
  }

  public function readSeparator(): void {
    if ($this->first) {
      $this->first = false;
      return;
    }

    $this->skipWhitespace();
    $c = $this->trans->readAll(1);
    $target = $this->colon ? ':' : ',';

    if ($c !== $target) {
      throw new TProtocolException(
        'TSimpleJSONProtocol: Expected "'.
        $target.
        '", encountered 0x'.
        bin2hex($c),
      );
    }

    $this->colon = !$this->colon;
  }

  public function readContextOver(): bool {
    $pos = $this->skipWhitespace(false);
    $c = $this->bufTrans->peek(1, $pos);
    if (!$this->first && $c !== ',' && $c !== '}') {
      throw new TProtocolException(
        'TSimpleJSONProtocol: Expected "," or "}", encountered 0x'.
        bin2hex($c),
      );
    }

    return ($c === '}');
  }

  public function readEnd(): void {
    $this->skipWhitespace();
    $c = $this->trans->readAll(1);
    if ($c !== '}') {
      throw new TProtocolException(
        'TSimpleJSONProtocol: Expected "}", encountered 0x'.bin2hex($c),
      );
    }
  }

  public function escapeNum(): bool {
    return $this->colon;
  }
}
