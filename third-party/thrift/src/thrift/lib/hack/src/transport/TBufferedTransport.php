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
 * Buffered transport. Stores data to an internal buffer that it doesn't
 * actually write out until flush is called. For reading, we do a greedy
 * read and then serve data out of the internal buffer.
 *
 * @package thrift.transport
 */
class TBufferedTransport extends TTransport
  implements TTransportStatus, IThriftBufferedTransport {

  /**
   * Constructor. Creates a buffered transport around an underlying transport
   */
  public function __construct(
    ?TTransport $transport = null,
    int $rBufSize = 512,
    int $wBufSize = 512,
  ) {
    $this->transport_ = $transport ?: new TNullTransport();
    $this->rBufSize_ = $rBufSize;
    $this->wBufSize_ = $wBufSize;
  }

  /**
   * The underlying transport
   *
   * @var TTransport
   */
  protected TTransport $transport_;

  /**
   * The receive buffer size
   *
   * @var int
   */
  protected int $rBufSize_ = 512;

  /**
   * The write buffer size
   *
   * @var int
   */
  protected int $wBufSize_ = 512;

  /**
   * The write buffer.
   *
   * @var string
   */
  protected string $wBuf_ = '';

  /**
   * The read buffer.
   *
   * @var string
   */
  protected string $rBuf_ = '';

  public function isOpen(): bool {
    return $this->transport_->isOpen();
  }

  public function open(): void {
    $this->transport_->open();
  }

  public function close(): void {
    $this->transport_->close();
  }

  public function putBack(string $data): void {
    if (strlen($this->rBuf_) === 0) {
      $this->rBuf_ = $data;
    } else {
      $this->rBuf_ = ($data.$this->rBuf_);
    }
  }

  public function getMetaData(): array<string, mixed> {
    if ($this->transport_ instanceof TSocket) {
      return $this->transport_->getMetaData();
    }

    return array();
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
    return strlen($this->rBuf_);
  }

  /**
   * The reason that we customize readAll here is that the majority of PHP
   * streams are already internally buffered by PHP. The socket stream, for
   * example, buffers internally and blocks if you call read with $len greater
   * than the amount of data available, unlike recv() in C.
   *
   * Therefore, use the readAll method of the wrapped transport inside
   * the buffered readAll.
   */
  public function readAll(int $len): string {
    $have = strlen($this->rBuf_);
    $data = '';
    if ($have == 0) {
      $data = $this->transport_->readAll($len);
    } else if ($have < $len) {
      $data = $this->rBuf_;
      $this->rBuf_ = '';
      $data .= $this->transport_->readAll($len - $have);
    } else if ($have == $len) {
      $data = $this->rBuf_;
      $this->rBuf_ = '';
    } else if ($have > $len) {
      $data = substr($this->rBuf_, 0, $len);
      $this->rBuf_ = substr($this->rBuf_, $len);
    }
    return $data;
  }

  public function read(int $len): string {
    if (strlen($this->rBuf_) === 0) {
      $this->rBuf_ = $this->transport_->read($this->rBufSize_);
    }

    if (strlen($this->rBuf_) <= $len) {
      $ret = $this->rBuf_;
      $this->rBuf_ = '';
      return $ret;
    }

    $ret = substr($this->rBuf_, 0, $len);
    $this->rBuf_ = substr($this->rBuf_, $len);
    return $ret;
  }

  /**
   * Peek some bytes in the buffer without removing the bytes from it
   *
   * @param int $len   length to peek
   * @param int $start the start position of the returned string
   *
   * @return null on peek failure
   */
  public function peek(int $len, int $start = 0): string {
    $bytes_needed = $len + $start;
    // read until either timeout OR get enough bytes
    if (strlen($this->rBuf_) == 0) {
      $this->rBuf_ = $this->transport_->readAll($bytes_needed);
    } else if ($bytes_needed > strlen($this->rBuf_)) {
      $this->rBuf_ .=
        $this->transport_->readAll($bytes_needed - strlen($this->rBuf_));
    }

    $ret = substr($this->rBuf_, $start, $len);
    return $ret;
  }

  public function write(string $buf): void {
    $this->wBuf_ .= $buf;
    if (strlen($this->wBuf_) >= $this->wBufSize_) {
      $out = $this->wBuf_;

      // Note that we clear the internal wBuf_ prior to the underlying write
      // to ensure we're in a sane state (i.e. internal buffer cleaned)
      // if the underlying write throws up an exception
      $this->wBuf_ = '';
      $this->transport_->write($out);
    }
  }

  public function flush(): void {
    if (strlen($this->wBuf_) > 0) {
      $this->transport_->write($this->wBuf_);
      $this->wBuf_ = '';
    }
    $this->transport_->flush();
  }

}

