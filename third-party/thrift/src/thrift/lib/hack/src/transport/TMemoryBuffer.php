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
 * A memory buffer is a tranpsort that simply reads from and writes to an
 * in-memory string buffer. Anytime you call write on it, the data is simply
 * placed into a buffer, and anytime you call read, data is read from that
 * buffer.
 *
 * @package thrift.transport
 */
<<Oncalls('thrift')>> // @oss-disable
final class TMemoryBuffer
  extends TWritePropsTransport
  implements IThriftBufferedTransport {

  private int $index_ = 0;
  private ?int $length_ = null;

  /**
   * Constructor. Optionally pass an initial value
   * for the buffer.
   */
  public function __construct(private string $buf_ = '')[] {}

  <<__Override>>
  public function isOpen()[]: bool {
    return true;
  }

  <<__Override>>
  public function open()[]: void {}

  <<__Override>>
  public function close()[]: void {}

  private function length()[write_props]: int {
    if ($this->length_ === null) {
      $this->length_ = Str\length($this->buf_);
    }
    return $this->length_;
  }

  public function available()[write_props]: int {
    return $this->length() - $this->index_;
  }

  <<__Override>>
  public function write(string $buf)[write_props]: void {
    $this->buf_ .= $buf;
    $this->length_ = null; // reset length
  }

  <<__Override>>
  public function read(int $len)[write_props]: string {
    $available = $this->available();
    if ($available === 0) {
      throw new TTransportException(
        'TMemoryBuffer: Could not read '.
        $len.
        ' bytes from buffer.'.
        ' Original length is '.
        $this->length().
        ' Current index is '.
        $this->index_,
        TTransportException::UNKNOWN,
      );
    }

    if ($available < $len) {
      $len = $available;
    }
    $ret = $this->peek($len);
    $this->index_ += $len;
    return $ret;
  }

  // This is the same as the parent implementation except the narrower coeffect.
  <<__Override>>
  public function readAll(int $len)[write_props]: string {
    $data = '';
    for ($got = Str\length($data); $got < $len; $got = Str\length($data)) {
      $data .= $this->read($len - $got);
    }
    return $data;
  }

  public function peek(int $len, int $start = 0)[]: string {
    if ($len !== 1) {
      return Str\slice($this->buf_, $this->index_ + $start, $len);
    }
    if (Str\length($this->buf_)) {
      return $this->buf_[$this->index_ + $start];
    }
    return '';
  }

  public function putBack(string $buf)[write_props]: void {
    if ($this->available() === 0) {
      $this->buf_ = $buf;
    } else {
      $remaining = (string)PHP\substr($this->buf_, $this->index_);
      $this->buf_ = $buf.$remaining;
    }
    $this->length_ = null;
    $this->index_ = 0;
  }

  public function getBuffer()[]: <<__Soft>> string {
    if ($this->index_ === 0) {
      return $this->buf_;
    }
    return PHP\substr($this->buf_, $this->index_);
  }

  public function resetBuffer()[write_props]: void {
    $this->buf_ = '';
    $this->index_ = 0;
    $this->length_ = null;
  }
}
