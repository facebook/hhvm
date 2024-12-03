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
 * Buffered transport. Stores data to an internal buffer that it doesn't
 * actually write out until flush is called. For reading, we do a greedy
 * read and then serve data out of the internal buffer.
 *
 * @package thrift.transport
 */
<<Oncalls('thrift')>> // @oss-disable
final class TBufferedTransport<<<__Soft>> reify TTrans as TTransport>
  extends TTransport
  implements TTransportStatus, IThriftBufferedTransport {

  /**
   * Constructor. Creates a buffered transport around an underlying transport
   *
   * @param $transport_ The underlying transport
   * @param $rBufSize_ The receive buffer size
   * @param $wBufSize_ The write buffer size
   */
  public function __construct(
    protected TTrans $transport_,
    protected int $rBufSize_ = 512,
    protected int $wBufSize_ = 512,
  )[] {}

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

  <<__Override>>
  public function isOpen()[]: bool {
    return $this->transport_->isOpen();
  }

  <<__Override>>
  public function open()[zoned_shallow]: void {
    $this->transport_->open();
  }

  <<__Override>>
  public function close()[zoned_shallow]: void {
    $this->transport_->close();
  }

  public function putBack(string $data)[write_props]: void {
    if (Str\length($this->rBuf_) === 0) {
      $this->rBuf_ = $data;
    } else {
      $this->rBuf_ = ($data.$this->rBuf_);
    }
  }

  public function getMetaData()[]: dict<string, mixed> {
    if ($this->transport_ is TSocket) {
      return $this->transport_->getMetaData();
    }

    return dict[];
  }

  public function isReadable()[leak_safe]: bool {
    if (Str\length($this->rBuf_) > 0) {
      return true;
    }

    if ($this->transport_ is TTransportStatus) {
      return $this->transport_->isReadable();
    }

    return true;
  }

  public function isWritable()[leak_safe]: bool {
    if ($this->transport_ is TTransportStatus) {
      return $this->transport_->isWritable();
    }

    return true;
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
  <<__Override>>
  public function readAll(int $len)[zoned_shallow]: string {
    $have = Str\length($this->rBuf_);
    $data = '';
    if ($have === 0) {
      $data = $this->transport_->readAll($len);
    } else if ($have < $len) {
      $data = $this->rBuf_;
      $this->rBuf_ = '';
      $data .= $this->transport_->readAll($len - $have);
    } else if ($have === $len) {
      $data = $this->rBuf_;
      $this->rBuf_ = '';
    } else if ($have > $len) {
      $data = PHP\substr($this->rBuf_, 0, $len);
      $this->rBuf_ = PHP\substr($this->rBuf_, $len);
    }
    return $data;
  }

  <<__Override>>
  public function read(int $len)[zoned_shallow]: string {
    if (Str\length($this->rBuf_) === 0) {
      $this->rBuf_ = $this->transport_->read($this->rBufSize_);
    }

    if (Str\length($this->rBuf_) <= $len) {
      $ret = $this->rBuf_;
      $this->rBuf_ = '';
      return $ret;
    }

    $ret = PHP\substr($this->rBuf_, 0, $len);
    $this->rBuf_ = PHP\substr($this->rBuf_, $len);
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
  public function peek(int $len, int $start = 0)[zoned_shallow]: string {
    $bytes_needed = $len + $start;
    // read until either timeout OR get enough bytes
    if (Str\length($this->rBuf_) === 0) {
      $this->rBuf_ = $this->transport_->readAll($bytes_needed);
    } else if ($bytes_needed > Str\length($this->rBuf_)) {
      $this->rBuf_ .= $this->transport_
        ->readAll($bytes_needed - Str\length($this->rBuf_));
    }

    $ret = PHP\substr($this->rBuf_, $start, $len);
    return $ret;
  }

  <<__Override>>
  public function write(string $buf)[zoned_shallow]: void {
    $this->wBuf_ .= $buf;
    if (Str\length($this->wBuf_) >= $this->wBufSize_) {
      $out = $this->wBuf_;

      // Note that we clear the internal wBuf_ prior to the underlying write
      // to ensure we're in a sane state (i.e. internal buffer cleaned)
      // if the underlying write throws up an exception
      $this->wBuf_ = '';
      $this->transport_->write($out);
    }
  }

  <<__Override>>
  public function flush()[zoned_shallow]: void {
    if (Str\length($this->wBuf_) > 0) {
      $this->transport_->write($this->wBuf_);
      $this->wBuf_ = '';
    }
    $this->transport_->flush();
  }

}
