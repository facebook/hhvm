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
 * NonBlocking implementation of TSocket. Does internal
 * buffering on sends and recvs. An external socket
 * select loop can drive io. (See ClientSet.php).
 *
 * @package thrift.transport
 */
class TNonBlockingSocket extends TTransport implements IThriftRemoteConn {

  /**
   * Handle to PHP socket
   *
   * @var resource
   */
  private ?resource $handle_ = null;

  /**
   * Remote hostname
   *
   * @var string
   */
  protected string $host_ = 'localhost';

  /**
   * Remote port
   *
   * @var int
   */
  protected int $port_ = 9090;

  protected bool $ipV6_ = false;

  /**
   * The write buffer.
   *
   *  @var string
   */
  protected string $wBuf_ = '';

  /**
   * The read buffer and pos.
   *
   *  @var string
   *  @var int
   */
  protected string $rBuf_ = '';
  protected int $rBufPos_ = 0;

  /**
   * Debug on?
   *
   * @var bool
   */
  protected bool $debug_ = false;

  /**
   * Debug handler
   *
   * @var mixed
   */
  protected (function(string): bool) $debugHandler_;

  /**
   * Socket recv buffer capacity.
   * @var int or null.
   */
  private ?int $sockRecvCapacity_ = null;

  /**
   * Socket constructor
   *
   * @param string $host         Remote hostname
   * @param int    $port         Remote port
   * @param string $debugHandler Function to call for error logging
   */

  public function __construct(
    string $host = 'localhost',
    int $port = 9090,
    ?(function(string): bool) $debugHandler = null,
  ) {
    $this->host_ = $host;
    $this->port_ = $port;
    $this->ipV6_ = strlen(@inet_pton($host)) == 16;

    $this->debugHandler_ = $debugHandler ?: fun('error_log');
  }

  /**
   * Get the host that this socket is connected to
   *
   * @return string host
   */
  public function getHost(): string {
    return $this->host_;
  }

  /**
   * Get the remote port that this socket is connected to
   *
   * @return int port
   */
  public function getPort(): int {
    return $this->port_;
  }

  /**
   * Get the socket for this connection. So we can select on it.
   *
   * @return socket resource
   */
  public function getSocket(): ?resource {
    return $this->handle_;
  }

  /**
   * Tests whether this is open
   *
   * @return bool true if the socket is open
   */
  public function isOpen(): bool {
    return is_resource($this->handle_);
  }

  /**
   * These methods are required for IThriftRemoteConn, but are not used
   */
  public function getRecvTimeout(): int {
    return 0;
  }

  public function setRecvTimeout(int $timeout): void {
    throw new TTransportException('setRecvTimeout is insupported');
  }

  public function isReadable(): bool {
    return $this->isOpen();
  }

  public function isWritable(): bool {
    return $this->isOpen();
  }

  /**
   * Set debugging
   *
   * @param bool on/off
   */
  public function setDebug(bool $debug): void {
    $this->debug_ = $debug;
  }

  /**
   * Connects the socket.
   */
  public function open(): void {
    if ($this->ipV6_) {
      $handle = socket_create(AF_INET6, SOCK_STREAM, SOL_TCP);
    } else {
      $handle = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
    }

    if ($handle === false) {
      $error = 'TNonBlockingSocket: Could not create socket';
      throw new TTransportException($error);
    }

    $this->handle_ = $handle;

    if (!socket_set_nonblock($this->handle_)) {
      $error = 'TNonBlockingSocket: Could not set nonblocking.';
      throw new TTransportException($error);
    }

    $res = @socket_connect(
      $this->handle_,
      $this->host_,
      $this->port_,
    );

    if (!$res) {
      $errno = socket_last_error($this->handle_);
      $errstr = socket_strerror($errno);
      $error =
        'TNonBlockingSocket: socket_connect error ('.
        (string) $errstr.'['.(string) $errno.'])';
      if ($errno != 115) {
        if ($this->debug_) {
          call_user_func($this->debugHandler_, $error);
        }
      }
      throw new TTransportException($error);
    }

    $wBuf_ = '';
    $rBuf_ = '';
    $rBufPos_ = 0;

    $this->sockRecvCapacity_ =
      socket_get_option($this->handle_, SOL_SOCKET, SO_RCVBUF);
    if ($this->sockRecvCapacity_ == false) {
      $this->sockRecvCapacity_ = null;
    }
  }

  /**
   * Closes the socket.
   */
  public function close(): void {
    if ($this->handle_ !== null) {
      @socket_close($this->handle_);
    }
    $this->handle_ = null;
  }

  /**
   * Do a nonblocking read. If all of $len is not there, throw an exception.
   * Save unread data to the recv buffer so subsequent reads can retrieve it.
   *
   * @param int $len How many bytes
   * @return string Binary data
   */
  public function readAll(int $len): string {
    // We may already have data for this read in the buffer
    $ret = (string) substr($this->rBuf_, $this->rBufPos_);
    // set the buffer to all but this read
    $this->rBuf_ = (string) substr($this->rBuf_, 0, $this->rBufPos_);

    if ($len <= strlen($ret)) {
      $this->rBuf_ .= $ret;
      $this->rBufPos_ += $len;

      return substr($ret, 0, $len);
    }

    // we already have this much for this read
    $len -= strlen($ret);

    while (true) {
      $buf = $this->read($len);

      if ($buf === '') {
        // Put back what we may have already read for this read.
        // Don't advance pos
        $this->rBuf_ .= $ret;

        throw new TTransportException(
          'TNonBlockingSocket: readAll could not'.
          ' read '.
          $len.
          ' bytes from '.
          $this->host_.
          ':'.
          $this->port_,
        );
      } else if (($sz = strlen($buf)) < $len) {
        $ret .= $buf;
        $len -= $sz;
      } else {
        $ret .= $buf;
        $this->rBuf_ .= $ret;
        $this->rBufPos_ += strlen($ret); // advance pos to next read

        return $ret;
      }
    }

    throw new TTransportException(
      "TNonBlockingSocket: You shouldn't be here",
    );
  }

  /**
   * Occasionally we will restart several readAlls due to a failure,
   * EAGAIN for instance, and we want to read all data we already buffered.
   * In this case call resetBufferPos. This occurs for example when a gen
   * client does several recevies, and fails on one other than the first.
   * It then retries all of them
   */
  public function resetBufferPos(): void {
    $this->rBufPos_ = 0;
  }

  /*
   * Clear the buffer and reset it
   */
  public function clearBuf(): void {
    $this->rBuf_ = '';
    $this->rBufPos_ = 0;
  }

  /**
   * Read from the socket
   *
   * @param int $len How many bytes
   * @return string Binary data
   */
  public function read(int $len): string {
    if ($this->sockRecvCapacity_ !== null) {
      $len = min($len, $this->sockRecvCapacity_);
    }

    $data = @socket_read($this->handle_, $len);

    if ($data === false || $data === '') {
      $errno = socket_last_error($this->handle_);
      $errstr = socket_strerror($errno);
      $error =
        "read: no data to be read ".
        $this->host_.
        ':'.
        $this->port_.
        ' ('.
        (string) $errstr.' ['.(string) $errno.'])';
      if ($this->debug_) {
        call_user_func($this->debugHandler_, $error);
      }

      return '';
    }

    return $data;
  }

  /**
   * Do a buffered write. Use doWrite to notify when the socket can be written.
   *
   * @param string $buf The data to write
   */
  public function write(string $buf): void {
    $this->wBuf_ .= $buf;
  }

  public function doWrite(): void {
    $got = @socket_write($this->handle_, $this->wBuf_);

    if ($got === 0 || $got === false) {
      // Could not write
      $errno = socket_last_error($this->handle_);
      $errstr = socket_strerror($errno);
      $error =
        'doWrite: write failed ('.
        (string) $errno.
        '): '.
        (string) $errstr.' '.$this->host_.':'.$this->port_;
      throw new TTransportException($error);
    }

    $this->wBuf_ = substr($this->wBuf_, $got);
  }

  /**
   * Do we have buffered data to send?
   *
   * @return bool
   */
  public function haveData(): bool {
    return strlen($this->wBuf_) > 0;
  }

  /**
   * No flush implemented.
   * Generated code will flush on send, we'd like to send as data becomes
   * available without blocking.
   */
  public function flush(): void {}
}
