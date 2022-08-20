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
 * @package thrift.transport
 */

/**
 * A memory buffer is a tranpsort that simply reads from and writes to an
 * in-memory string buffer. Anytime you call write on it, the data is simply
 * placed into a buffer, and anytime you call read, data is read from that
 * buffer.
 *
 * @package thrift.transport
 */
class TMemoryBuffer extends TTransport implements IThriftBufferedTransport {

  private string $buf_ = '';
  private int $index_ = 0;
  private ?int $length_ = null;

  /**
   * Constructor. Optionally pass an initial value
   * for the buffer.
   */
  public function __construct(string $buf = '') {
    $this->buf_ = (string) $buf;
  }

  public function isOpen(): bool {
    return true;
  }

  public function open(): void {}

  public function close(): void {}

  private function length(): int {
    if ($this->length_ === null) {
      $this->length_ = strlen($this->buf_);
    }
    return $this->length_;
  }

  public function available(): int {
    return $this->length() - $this->index_;
  }

  public function minBytesAvailable(): int {
    return $this->available();
  }

  public function write(string $buf): void {
    $this->buf_ .= $buf;
    $this->length_ = null; // reset length
  }

  public function read(int $len): string {
    $available = $this->available();
    if ($available === 0) {
      $buffer_dump = bin2hex($this->buf_);
      throw new TTransportException(
        'TMemoryBuffer: Could not read '.
        $len.
        ' bytes from buffer.'.
        ' Original length is '.
        $this->length().
        ' Current index is '.
        $this->index_.
        ' Buffer content <start>'.
        $buffer_dump.
        '<end>',
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

  public function peek(int $len, int $start = 0): string {
    return
      $len === 1
        ? $this->buf_[$this->index_ + $start]
        : substr($this->buf_, $this->index_ + $start, $len);
  }

  public function putBack(string $buf): void {
    if ($this->available() === 0) {
      $this->buf_ = $buf;
    } else {
      $remaining = (string) substr($this->buf_, $this->index_);
      $this->buf_ = $buf.$remaining;
    }
    $this->length_ = null;
    $this->index_ = 0;
  }

  public function getBuffer(): @string { // Task #5347782
    if ($this->index_ === 0) {
      return $this->buf_;
    }
    return substr($this->buf_, $this->index_);
  }

  public function resetBuffer(): void {
    $this->buf_ = '';
    $this->index_ = 0;
    $this->length_ = null;
  }
}
