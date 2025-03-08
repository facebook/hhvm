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

abstract class TWritePropsTransport extends TTransport {
  <<__Override>>
  public abstract function open()[write_props]: void;

  <<__Override>>
  public abstract function close()[write_props]: void;

  <<__Override>>
  public abstract function read(int $len)[write_props]: string;

  <<__Override>>
  public abstract function write(string $buf)[write_props]: void;

  <<__Override>>
  public function readAll(int $len)[write_props]: string {
    // return $this->read($len);
    $data = '';
    for ($got = Str\length($data); $got < $len; $got = Str\length($data)) {
      $data .= $this->read($len - $got);
    }
    return $data;
  }

  <<__Override>>
  public function flush()[write_props]: void {}

  <<__Override>>
  public function onewayFlush()[write_props]: void {
    // Default to flush()
    $this->flush();
  }
}
