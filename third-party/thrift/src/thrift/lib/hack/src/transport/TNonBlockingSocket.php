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
 * NonBlocking implementation of TSocket. Does internal
 * buffering on sends and recvs. An external socket
 * select loop can drive io. (See ClientSet.php).
 *
 * @package thrift.transport
 */
<<Oncalls('thrift')>> // @oss-disable
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

  public function __construct(string $host = 'localhost', int $port = 9090)[] {
    $this->host_ = $host;
    $this->port_ = $port;
    $this->ipV6_ = Str\contains($host, ':') &&
      PHP\filter_var($host, FILTER_VALIDATE_IP, FILTER_FLAG_IPV6) !== false;
  }

  /**
   * Get the host that this socket is connected to
   *
   * @return string host
   */
  public function getHost()[]: string {
    return $this->host_;
  }

  /**
   * Get the remote port that this socket is connected to
   *
   * @return int port
   */
  public function getPort()[]: int {
    return $this->port_;
  }

  /**
   * Get the socket for this connection. So we can select on it.
   *
   * @return socket resource
   */
  public function getSocket()[]: ?resource {
    return $this->handle_;
  }

  /**
   * Tests whether this is open
   *
   * @return bool true if the socket is open
   */
  <<__Override>>
  public function isOpen()[]: bool {
    return $this->handle_ is resource;
  }

  /**
   * These methods are required for IThriftRemoteConn, but are not used
   */
  <<__Deprecated(
    'This function was found unused by CodemodRuleDeprecateUnusedClassMethod',
  )>>
  public function getRecvTimeout()[]: int {
    return 0;
  }

  public function setRecvTimeout(int $timeout)[]: void {
    throw new TTransportException('setRecvTimeout is insupported');
  }

  public function isReadable()[]: bool {
    return $this->isOpen();
  }

  public function isWritable()[]: bool {
    return $this->isOpen();
  }

  /**
   * Connects the socket.
   */
  <<__Override>>
  public function open()[leak_safe]: void {
    $handle = (
      () ==> {
        if ($this->ipV6_) {
          return PHP\socket_create(AF_INET6, SOCK_STREAM, SOL_TCP);
        } else {
          return PHP\socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
        }
      }
    )();

    if ($handle === false) {
      $error = 'TNonBlockingSocket: Could not create socket';
      throw new TTransportException($error);
    }

    $this->handle_ = $handle;

    if (
      !HH\Coeffects\fb\backdoor_from_leak_safe__DO_NOT_USE(
        ()[defaults] ==> PHP\socket_set_nonblock($this->handle_ as nonnull),
        'Waiting for D32856835.',
      )
    ) {
      $error = 'TNonBlockingSocket: Could not set nonblocking.';
      throw new TTransportException($error);
    }/* BEGIN_STRIP */

    WallTimeProfiler::pushInstance();
    $timer = WallTimeOperation::begin();
    WallTimeProfiler::get()->beginIOWait();
    /* END_STRIP */

    $res = PHPism_FIXME::suppressAllErrors(
      ()[leak_safe] ==> HH\Coeffects\fb\backdoor_from_leak_safe__DO_NOT_USE(
        ()[defaults] ==> PHP\socket_connect(
          $this->handle_ as nonnull,
          $this->host_,
          $this->port_,
        ),
        'Waiting for D32856835.',
      ),
    );/* BEGIN_STRIP */
    WallTimeProfiler::get()->endIOWait(null);
    $timer->end();
    WallTimeProfiler::popInstance();

    $this->incrCount(ProfilingCounterCount::THRIFT_OPEN_COUNT);
    $this->incrDuration(
      ProfilingCounterDuration::THRIFT_OPEN_DURATION,
      $timer->getOldestRunningIODuration(),
    );
    /* END_STRIP */

    if (!$res) {
      $errno = PHP\socket_last_error($this->handle_);
      $errstr = PHP\socket_strerror($errno);
      $error = 'TNonBlockingSocket: socket_connect error ('.
        (string)$errstr.
        '['.
        (string)$errno.
        '])';
      throw new TTransportException($error);
    }

    $this->sockRecvCapacity_ =
      PHP\socket_get_option($this->handle_ as nonnull, SOL_SOCKET, SO_RCVBUF);
    if ($this->sockRecvCapacity_ === false) {
      $this->sockRecvCapacity_ = null;
    }
  }

  /**
   * Closes the socket.
   */
  <<__Override>>
  public function close()[leak_safe]: void {
    if ($this->handle_ !== null) {
      PHPism_FIXME::suppressAllErrors(
        ()[write_props] ==> (
          () ==> PHP\socket_close($this->handle_ as nonnull)
        )(),
      );
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
  <<__Override>>
  public function readAll(int $len)[zoned_shallow]: string {
    // We may already have data for this read in the buffer
    $ret = (string)PHP\substr($this->rBuf_, $this->rBufPos_);
    // set the buffer to all but this read
    $this->rBuf_ = (string)PHP\substr($this->rBuf_, 0, $this->rBufPos_);

    if ($len <= Str\length($ret)) {
      $this->rBuf_ .= $ret;
      $this->rBufPos_ += $len;

      return PHP\substr($ret, 0, $len);
    }

    // we already have this much for this read
    $len -= Str\length($ret);

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
      } else {
        $sz = Str\length($buf);
        if ($sz < $len) {
          $ret .= $buf;
          $len -= $sz;
        } else {
          $ret .= $buf;
          $this->rBuf_ .= $ret;
          $this->rBufPos_ += Str\length($ret); // advance pos to next read

          return $ret;
        }
      }
    }

    throw new TTransportException("TNonBlockingSocket: You shouldn't be here");
  }

  /**
   * Occasionally we will restart several readAlls due to a failure,
   * EAGAIN for instance, and we want to read all data we already buffered.
   * In this case call resetBufferPos. This occurs for example when a gen
   * client does several recevies, and fails on one other than the first.
   * It then retries all of them
   */
  public function resetBufferPos()[write_props]: void {
    $this->rBufPos_ = 0;
  }

  /*
   * Clear the buffer and reset it
   */
  public function clearBuf()[write_props]: void {
    $this->rBuf_ = '';
    $this->rBufPos_ = 0;
  }

  /**
   * Read from the socket
   *
   * @param int $len How many bytes
   * @return string Binary data
   */
  <<__Override>>
  public function read(int $len)[zoned_shallow]: string {
    if ($this->sockRecvCapacity_ !== null) {
      $len = Math\minva($len, $this->sockRecvCapacity_);
    }/* BEGIN_STRIP */

    WallTimeProfiler::pushInstance();
    $timer = WallTimeOperation::begin();
    WallTimeProfiler::get()->beginIOWait();
    /* END_STRIP */

    $data = PHPism_FIXME::suppressAllErrors(
      ()[zoned_shallow] ==> $this->socketRead($len),
    );/* BEGIN_STRIP */

    WallTimeProfiler::get()->endIOWait(null);
    $timer->end();
    WallTimeProfiler::popInstance();

    $this->incrCount(ProfilingCounterCount::THRIFT_READ_COUNT);
    $this->incrCount(
      ProfilingCounterCount::THRIFT_READ_BYTES,
      Str\length($data),
    );
    $this->incrDuration(
      ProfilingCounterDuration::THRIFT_READ_DURATION,
      $timer->getOldestRunningIODuration(),
    );
    /* END_STRIP */

    if ($data === '') {
      return '';
    }

    return $data;
  }

  public function socketRead(int $len)[zoned_shallow]: string {
    $data = HH\Coeffects\fb\backdoor_from_write_props__DO_NOT_USE(
      ()[defaults] ==> PHP\socket_read($this->handle_ as nonnull, $len),
      'Expecting a policy check higher in the stack. Backdoor for now '.
      '(T107484356).',
    );
    return $data !== false ? $data as string : '';
  }

  /**
   * Do a buffered write. Use doWrite to notify when the socket can be written.
   *
   * @param string $buf The data to write
   */
  <<__Override>>
  public function write(string $buf)[write_props]: void {
    $this->wBuf_ .= $buf;
  }

  public function doWrite()[zoned_shallow]: void {/* BEGIN_STRIP */
    WallTimeProfiler::pushInstance();
    $timer = WallTimeOperation::begin();
    WallTimeProfiler::get()->beginIOWait();
    /* END_STRIP */
    $got = PHPism_FIXME::suppressAllErrors(
      ()[zoned_shallow] ==> $this->socketWrite($this->wBuf_),
    );/* BEGIN_STRIP */

    WallTimeProfiler::get()->endIOWait(null);
    $timer->end();
    WallTimeProfiler::popInstance();

    $this->incrCount(ProfilingCounterCount::THRIFT_WRITE_COUNT);
    $this->incrCount(ProfilingCounterCount::THRIFT_WRITE_BYTES, (int)$got);
    $this->incrDuration(
      ProfilingCounterDuration::THRIFT_WRITE_DURATION,
      $timer->getOldestRunningIODuration(),
    );
    /* END_STRIP */

    if ($got === 0 || $got === false) {
      // Could not write
      $errno = PHP\socket_last_error($this->handle_);
      $errstr = PHP\socket_strerror($errno);
      $error = 'doWrite: write failed ('.
        (string)$errno.
        '): '.
        (string)$errstr.
        ' '.
        $this->host_.
        ':'.
        $this->port_;
      throw new TTransportException($error);
    }

    $this->wBuf_ = Str\slice($this->wBuf_, $got);
  }

  public function socketWrite(string $buf)[zoned_shallow]: dynamic {
    return HH\Coeffects\fb\backdoor_from_write_props__DO_NOT_USE(
      ()[defaults] ==> PHP\socket_write($this->handle_ as nonnull, $buf),
      'Expecting a policy check higher in the stack. Backdoor for now '.
      '(T107484356).',
    );
  }

  /**
   * Do we have buffered data to send?
   *
   * @return bool
   */
  public function haveData()[]: bool {
    return Str\length($this->wBuf_) > 0;
  }

  /**
   * No flush implemented.
   * Generated code will flush on send, we'd like to send as data becomes
   * available without blocking.
   */
  <<__Override>>
  public function flush()[]: void {}/* BEGIN_STRIP */

  /**
   * Name of the transport (e.g.: socket).
   */
  <<__Override>>
  public function getTransportType()[]: string {
    return 'non-blocking-socket';
  }
  /* END_STRIP */
}
