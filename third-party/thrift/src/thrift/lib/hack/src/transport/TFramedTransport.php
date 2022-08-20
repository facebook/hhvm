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
 * Framed transport. Writes and reads data in chunks that are stamped with
 * their length.
 *
 * @package thrift.transport
 */
class TFramedTransport extends TTransport
  implements TTransportStatus, IThriftBufferedTransport {

  /**
   * Underlying transport object.
   *
   * @var TTransport
   */
  protected TTransport $transport_;

  /**
   * Buffer for read data.
   *
   * @var string
   */
  protected string $rBuf_ = '';

  /**
   * Position in rBuf_ to read the next char
   *
   * @var int
   */
  protected int $rIndex_ = 0;

  /**
   * Buffer for queued output data
   *
   * @var string
   */
  protected string $wBuf_ = '';

  /**
   * Whether to frame reads
   *
   * @var bool
   */
  private bool $read_;

  /**
   * Whether to frame writes
   *
   * @var bool
   */
  private bool $write_;

  /**
   * Constructor.
   *
   * @param TTransport $transport Underlying transport
   */
  public function __construct(
    ?TTransport $transport = null,
    bool $read = true,
    bool $write = true,
  ) {
    $this->transport_ = $transport ?: new TNullTransport();
    $this->read_ = $read;
    $this->write_ = $write;
  }

  public function isOpen(): bool {
    return $this->transport_->isOpen();
  }

  public function open(): void {
    $this->transport_->open();
  }

  public function close(): void {
    $this->transport_->close();
  }

  public function isReadable(): bool {
    if (strlen($this->rBuf_) > 0) {
      return true;
    }

    if ($this->transport_ instanceof TTransportStatus) {
      return $this->transport_->isReadable();
    }

    return true;
  }

  public function isWritable(): bool {
    if ($this->transport_ instanceof TTransportStatus) {
      return $this->transport_->isWritable();
    }

    return true;
  }

  public function minBytesAvailable(): int {
    return strlen($this->rBuf_) - $this->rIndex_;
  }

  /**
   * Reads from the buffer. When more data is required reads another entire
   * chunk and serves future reads out of that.
   *
   * @param int $len How much data
   */
  public function read(int $len): string {
    if (!$this->read_) {
      return $this->transport_->read($len);
    }

    if (strlen($this->rBuf_) === 0) {
      $this->readFrame();
    }

    // Return substr
    $out = substr($this->rBuf_, $this->rIndex_, $len);
    $this->rIndex_ += $len;

    if (strlen($this->rBuf_) <= $this->rIndex_) {
      $this->rBuf_ = '';
      $this->rIndex_ = 0;
    }
    return $out;
  }

  /**
   * Peek some bytes in the frame without removing the bytes from the buffer
   *
   * @param int $len   length to peek
   * @param int $start the start position of the returned string
   */
  public function peek(int $len, int $start = 0): string {
    if (!$this->read_) {
      return '';
    }

    if (strlen($this->rBuf_) === 0) {
      $this->readFrame();
    }

    // Return substr
    $out = substr($this->rBuf_, $this->rIndex_ + $start, $len);

    return $out;
  }

  /**
   * Put previously read data back into the buffer
   *
   * @param string $data data to return
   */
  public function putBack(string $data): void {
    if (strlen($this->rBuf_) === 0) {
      $this->rBuf_ = $data;
    } else {
      $this->rBuf_ = ($data.(string) substr($this->rBuf_, $this->rIndex_));
    }
    $this->rIndex_ = 0;
  }

  /**
   * Reads a chunk of data into the internal read buffer.
   */
  private function readFrame(): void {
    $buf = $this->transport_->readAll(4);
    $val = unpack('N', $buf);
    $sz = $val[1];

    $this->rBuf_ = $this->transport_->readAll($sz);
    $this->rIndex_ = 0;
  }

  /**
   * Writes some data to the pending output buffer.
   *
   * @param string $buf The data
   * @param int    $len Limit of bytes to write
   */
  public function write(string $buf, ?int $len = null): void {
    if (!$this->write_) {
      $this->transport_->write($buf);
      return;
    }

    if ($len !== null && $len < strlen($buf)) {
      $buf = substr($buf, 0, $len);
    }
    $this->wBuf_ .= $buf;
  }

  /**
   * Writes the output buffer to the stream in the format of a 4-byte length
   * followed by the actual data.
   */
  public function flush(): void {
    if (!$this->write_ || strlen($this->wBuf_) == 0) {
      $this->transport_->flush();
      return;
    }

    $out = (string) pack('N', strlen($this->wBuf_));
    $out .= $this->wBuf_;

    // Note that we clear the internal wBuf_ prior to the underlying write
    // to ensure we're in a sane state (i.e. internal buffer cleaned)
    // if the underlying write throws up an exception
    $this->wBuf_ = '';
    $this->transport_->write($out);
    $this->transport_->flush();
  }

}
