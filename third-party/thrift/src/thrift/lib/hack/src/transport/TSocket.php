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
 * Sockets implementation of the TTransport interface.
 *
 * @package thrift.transport
 */
<<Oncalls('thrift')>> // @oss-disable
class TSocket
  extends TTransport
  implements TTransportStatus, InstrumentedTTransport, IThriftRemoteConn {

  use InstrumentedTTransportTrait;

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

  /**
   * Local port
   *
   * @var int
   */
  protected DisableRuntimeTypecheck<int> $lport_ = 0;

  /**
   * Send timeout in milliseconds
   *
   * @var int
   */
  private int $sendTimeout_ = 100;

  /**
   * Recv timeout in milliseconds
   *
   * @var int
   */
  private int $recvTimeout_ = 750;

  /**
   * Is send timeout set?
   *
   * @var bool
   */
  private bool $sendTimeoutSet_ = false;

  /**
   * Persistent socket or plain?
   *
   * @var bool
   */
  private bool $persist_ = false;

  /**
   * When the current read is started
   *
   * @var int, null means no read is started
   */
  private ?int $readAttemptStart_ = null;

  /**
   * When the current write is started
   *
   * @var int, null means no write is started
   */
  private ?int $writeAttemptStart_ = null;

  /**
   * error string (in case of open failure)
   *
   * @var string or null
   */
  protected ?DisableRuntimeTypecheck<string> $errstr_ = null;

  /**
   * error number (in case of open failure)
   *
   * @var int or null
   */
  protected ?DisableRuntimeTypecheck<int> $errno_ = null;

  /**
   * Specifies the maximum number of bytes to read
   * at once from internal stream.
   */
  protected ?int $maxReadChunkSize_ = null;

  /**
   * Socket constructor
   *
   * @param string $host         Remote hostname
   * @param int    $port         Remote port
   * @param bool   $persist      Whether to use a persistent socket
   */
  public function __construct(
    string $host = 'localhost',
    int $port = 9090,
    bool $persist = false,
  )[leak_safe] {/* BEGIN_STRIP */
    if (ip_is_valid($host)) {
      $this->host_ = IPAddress($host)->forURL();
    } else {
      // probably a hostname
      /* END_STRIP */
      $this->host_ = $host;/* BEGIN_STRIP */
    }
    /* END_STRIP */
    $this->port_ = $port;
    $this->persist_ = $persist;
  }

  /**
   * Sets the internal max read chunk size.
   * null for no limit (default).
   */
  public function setMaxReadChunkSize(
    int $maxReadChunkSize,
  )[write_props]: void {
    $this->maxReadChunkSize_ = $maxReadChunkSize;
  }

  /**
   * Sets the socket handle
   * @param resource $handle
   * @return $this
   */
  public function setHandle(resource $handle)[write_props]: this {
    $this->handle_ = $handle;
    return $this;
  }

  /**
   * Gets the meta_data for the current handle
   */
  public function getMetaData()[]: dict<string, mixed> {
    return HH\FIXME\UNSAFE_CAST<dynamic, dict<string, mixed>>(
      PHP\stream_get_meta_data($this->handle_ as nonnull),
    );
  }

  public function setTimeout(int $sec, int $msec)[leak_safe]: void {
    PHP\stream_set_timeout($this->handle_ as nonnull, $sec, $msec);
  }

  /**
   * Gets the send timeout.
   *
   * @return int timeout
   */
  public function getSendTimeout()[]: int {
    return $this->sendTimeout_;
  }

  /**
   * Sets the send timeout.
   *
   * @param int $timeout  Timeout in milliseconds.
   */
  public function setSendTimeout(int $timeout)[write_props]: void {
    $this->sendTimeout_ = $timeout;
  }

  /**
   * Gets the receive timeout.
   *
   * @return int timeout
   */
  public function getRecvTimeout()[]: int {
    return $this->recvTimeout_;
  }

  /**
   * Sets the receive timeout.
   *
   * @param int $timeout  Timeout in milliseconds.
   */
  public function setRecvTimeout(int $timeout)[write_props]: void {
    $this->recvTimeout_ = $timeout;
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
   * Get the error string in case of open failure
   *
   * @return errstr_ or null
   */
  public function getErrStr()[]: ?string {
    return $this->errstr_;
  }

  /**
   * Get the error number in case of open failure
   *
   * @return errno_ or null
   */
  public function getErrNo()[]: ?int {
    return $this->errno_;
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
   * Connects the socket.
   */
  <<__Override>>
  public function open()[leak_safe]: void {
    if ($this->isOpen()) {
      throw new TTransportException(
        'TSocket: socket already connected',
        TTransportException::ALREADY_OPEN,
      );
    }

    if ($this->port_ <= 0) {
      throw new TTransportException(
        'TSocket: cannot open without port',
        TTransportException::NOT_OPEN,
      );
    }/* BEGIN_STRIP */

    WallTimeProfiler::pushInstance();
    $timer = WallTimeOperation::begin();
    WallTimeProfiler::get()->beginIOWait();
    /* END_STRIP */

    if ($this->persist_) {
      $__errno = $this->errno_;
      $__errstr = $this->errstr_;
      $handle = HH\Coeffects\fb\backdoor_from_leak_safe__DO_NOT_USE(
        ()[defaults] ==> PHPism_FIXME::pfsockopen(
          $this->host_,
          $this->port_,
          inout $__errno,
          inout $__errstr,
          $this->sendTimeout_ / 1000.0,
        ),
        'Blocked by a migration of builtins_network to coeffects (T107312346).',
      );
      $this->errstr_ = $__errstr;
      $this->errno_ = $__errno;
    } else {
      $__errno = $this->errno_;
      $__errstr = $this->errstr_;
      $handle = HH\Coeffects\fb\backdoor_from_leak_safe__DO_NOT_USE(
        ()[defaults] ==> PHPism_FIXME::fsockopen(
          $this->host_,
          $this->port_,
          inout $__errno,
          inout $__errstr,
          $this->sendTimeout_ / 1000.0,
        ),
        'Blocked by a migration of builtins_network to coeffects (T107312346).',
      );
      $this->errstr_ = $__errstr;
      $this->errno_ = $__errno;
    }/* BEGIN_STRIP */
    WallTimeProfiler::get()->endIOWait(null);
    $timer->end();
    WallTimeProfiler::popInstance();

    $this->incrCount(ProfilingCounterCount::THRIFT_OPEN_COUNT);
    $this->incrDuration(
      ProfilingCounterDuration::THRIFT_OPEN_DURATION,
      $timer->getOldestRunningIODuration(),
    );
    /* END_STRIP */

    // Connect failed?
    if (!$handle) {
      $error = 'TSocket: could not connect to '.$this->host_.':'.$this->port_;
      $error .= ' ('.(string)$this->errstr_.' ['.(string)$this->errno_.'])';
      throw
        new TTransportException($error, TTransportException::COULD_NOT_CONNECT);
    }

    $this->handle_ = $handle;

    $sock_name = PHP\stream_socket_get_name($this->handle_ as nonnull, false);
    // IPv6 is returned [2401:db00:20:702c:face:0:7:0]:port
    // or when stream_socket_get_name is buggy it is
    // 2401:db00:20:702c:face:0:7:0:port
    $this->lport_ = HH\FIXME\UNSAFE_CAST<dynamic, int>(
      PHP\fb\end(Str\split($sock_name, ":")),
      'Exposed when typing PHP\fb\end',
    );

    $this->setTimeout(0, $this->sendTimeout_ * 1000);
    $this->sendTimeoutSet_ = true;
  }

  /**
   * Closes the socket.
   */
  <<__Override>>
  public function close()[leak_safe]: void {
    if (!$this->persist_ && $this->handle_ !== null) {
      HH\Coeffects\fb\backdoor_from_leak_safe__DO_NOT_USE(
        ()[defaults] ==> PHPism_FIXME::fclose($this->handle_ as nonnull),
        'Blocked by a migration of builtins_file to coeffects (T107309662).',
      );
      $this->handle_ = null;
    }
  }

  /**
   *  Test to see if the socket is ready for reading. This method returns
   *  immediately. If calling this method in a loop one should sleep or do
   *  other work else CPU cycles will be wasted.
   *
   *  @return bool  True if a non-blocking read of at least one character can
   *                be preformed on the socket.
   */
  public function isReadable()[leak_safe]: bool {
    return $this->isSocketActionable($this->handle_, /* $check_read */ true);
  }

  /**
   *  Test to see if the socket is ready for writing. This method returns
   *  immediately. If calling this method in a loop one should sleep or do
   *  other work else CPU cycles will be wasted.
   *
   *  @return bool True if a non-blocking write can be preformed on the socket.
   */
  public function isWritable()[leak_safe]: bool {
    $writable = $this->isSocketActionable(
      $this->handle_, /* $check_read */
      false,
    );
    if (!$writable && $this->sendTimeout_ > 0) {
      if ($this->writeAttemptStart_ === null) {
        $this->writeAttemptStart_ = PHP\microtime(true);
      }
      if (
        PHP\microtime(true) - (int)$this->writeAttemptStart_ >
          ($this->sendTimeout_ / 1000.0)
      ) {
        throw new TTransportException(
          'TSocket: socket not writable after '.$this->sendTimeout_.'ms',
          TTransportException::TIMED_OUT,
        );
      }
    }
    return $writable;
  }

  private function isSocketActionable(
    ?resource $socket,
    bool $check_read,
  )[leak_safe]: bool {
    // the socket is technically actionable, although any read or write will
    // fail since close() was already called.
    if ($socket === null) {
      return true;
    }

    $read = vec[];
    $write = $read;
    if ($check_read) {
      $read = vec[$socket];
    } else {
      $write = vec[$socket];
    }

    $excpt = vec[];
    $ret = (
      () ==> PHP\stream_select(inout $read, inout $write, inout $excpt, 0, 0)
    )();
    if ($ret === false) {
      $error = 'TSocket: stream_select failed on socket.';
      throw new TTransportException($error);
    }

    return $ret !== 0;
  }

  /**
   * Reads maximum min($len, $maxReadChunkSize_) bytes
   * from the stream.
   */
  private function readChunk(int $len)[zoned_shallow]: ?string {
    if ($this->maxReadChunkSize_ !== null) {
      $len = Math\minva($len, $this->maxReadChunkSize_);
    }/* BEGIN_STRIP */
    WallTimeProfiler::pushInstance();
    $timer = WallTimeOperation::begin();
    WallTimeProfiler::get()->beginIOWait();
    /* END_STRIP */

    $res = call_defaults_from_zoned_shallow(
      ()[defaults] ==> PHPism_FIXME::fread($this->handle_ as nonnull, $len),
      'Blocked by a migration of builtins_file to coeffects (T107309662).',
    );
    $size = Str\length($res);/* BEGIN_STRIP */

    WallTimeProfiler::get()->endIOWait(null);
    $timer->end();
    WallTimeProfiler::popInstance();
    if ($res === false) {
      return null;
    }

    $this->incrCount(ProfilingCounterCount::THRIFT_READ_COUNT);
    $this->incrCount(ProfilingCounterCount::THRIFT_READ_BYTES, $size);
    $this->incrDuration(
      ProfilingCounterDuration::THRIFT_READ_DURATION,
      $timer->getOldestRunningIODuration(),
    );
    /* END_STRIP */

    $this->onRead($size);
    return $res;
  }

  /**
   * Uses stream get contents to do the reading
   *
   * @param int $len How many bytes
   * @return string Binary data
   */
  <<__Override>>
  public function readAll(int $len)[zoned_shallow]: string {
    if ($this->sendTimeoutSet_) {
      $sec = 0;
      if ($this->recvTimeout_ > 1000) {
        $msec = $this->recvTimeout_ % 1000;
        $sec = (int)(($this->recvTimeout_ - $msec) / 1000);
      } else {
        $msec = $this->recvTimeout_;
      }
      $this->setTimeout($sec, $msec * 1000);
      $this->sendTimeoutSet_ = false;
    }
    // This call does not obey stream_set_timeout values!
    // $buf = @stream_get_contents($this->handle_, $len);
    $pre = '';
    while (true) {
      $t_start = PHP\microtime(true);
      $buf = $this->readChunk($len);
      $t_stop = PHP\microtime(true);
      if ($buf === null || $buf === '') {
        $read_err_detail = Str\format(
          '%d bytes from %s:%d to localhost:%d. Spent %2.2f ms.',
          $len,
          $this->host_,
          $this->port_,
          $this->lport_,
          HH\FIXME\UNSAFE_CAST<mixed, float>(
            ($t_stop - $t_start) * 1000,
            'Exposed by typing PHP\microtime',
          ),
        );
        $md = $this->getMetaData();
        if ($md['timed_out']) {
          throw new TTransportException(
            'TSocket: timeout while reading '.$read_err_detail,
            TTransportException::TIMED_OUT,
          );
        } else {
          throw new TTransportException(
            'TSocket: could not read '.$read_err_detail,
            TTransportException::COULD_NOT_READ,
          );
        }
      } else {
        $sz = Str\length($buf);
        if ($sz < $len) {
          $md = $this->getMetaData();
          if ($md['timed_out']) {
            $read_err_detail = Str\format(
              '%d bytes from %s:%d to localhost:%d. Spent %2.2f ms.',
              $len,
              $this->host_,
              $this->port_,
              $this->lport_,
              HH\FIXME\UNSAFE_CAST<mixed, float>(
                ($t_stop - $t_start) * 1000,
                'Exposed by typing PHP\microtime',
              ),
            );
            throw new TTransportException(
              'TSocket: timeout while reading '.$read_err_detail,
              TTransportException::TIMED_OUT,
            );
          } else {
            $pre .= $buf;
            $len -= $sz;
          }
        } else {
          $this->readAttemptStart_ = null;
          $res = $pre.$buf;
          $this->onRead(Str\length($res));
          return $res;
        }
      }
    }

    throw new TTransportException("TSocket: You shouldn't be here");
  }

  /**
   * Read from the socket
   *
   * @param int $len How many bytes
   * @return string Binary data
   */
  <<__Override>>
  public function read(int $len)[zoned_shallow]: string {
    if ($this->sendTimeoutSet_) {
      $this->setTimeout(0, $this->recvTimeout_ * 1000);
      $this->sendTimeoutSet_ = false;
    }
    $t_start = PHP\microtime(true);
    $data = $this->readChunk($len);
    $t_stop = PHP\microtime(true);
    if ($data === null || $data === '') {
      $read_err_detail = Str\format(
        '%d bytes from %s:%d to localhost:%d. Spent %2.2f ms.',
        $len,
        $this->host_,
        $this->port_,
        $this->lport_,
        HH\FIXME\UNSAFE_CAST<mixed, float>(
          ($t_stop - $t_start) * 1000,
          'Exposed by typing PHP\microtime',
        ),
      );
      $md = $this->getMetaData();
      if ($md['timed_out']) {
        throw new TTransportException(
          'TSocket: timeout while reading '.$read_err_detail,
          TTransportException::TIMED_OUT,
        );
      } else {
        throw new TTransportException(
          'TSocket: could not read '.$read_err_detail,
          TTransportException::COULD_NOT_READ,
        );
      }
    } else {
      $this->readAttemptStart_ = null;
    }

    $this->onRead(Str\length($data));
    return $data;
  }

  /**
   * Perform a nonblocking read.
   * @param int $len Number of bytes to read
   * @return string Binary data or '' is no data is read
   */
  public function nonBlockingRead(int $len)[zoned_shallow]: string {
    $md = $this->getMetaData();
    $is_blocking = $md['blocked'];

    // If the stream is currently blocking, we will set to nonblocking
    // first
    if (
      $is_blocking && !PHP\stream_set_blocking($this->handle_ as nonnull, false)
    ) {
      throw
        new TTransportException('TSocket: cannot set stream to non-blocking');
    }

    $data = $this->readChunk($len);

    if ($data === null) {
      throw new TTransportException('TSocket: failed in non-blocking read');
    }

    // Switch back to blocking mode is necessary
    if (
      $is_blocking && !PHP\stream_set_blocking($this->handle_ as nonnull, true)
    ) {
      throw new TTransportException(
        'TSocket: cannot swtich stream back to blocking',
      );
    }
    $this->onRead(Str\length($data));
    return $data;
  }

  /**
   * Write to the socket.
   *
   * @param string $buf The data to write
   */
  <<__Override>>
  public function write(string $buf)[zoned_shallow]: void {
    if ($this->handle_ === null) {
      throw new TException('TSocket: handle_ is null');
    }

    $this->onWrite(Str\length($buf));

    if (!$this->sendTimeoutSet_) {
      $this->setTimeout(0, $this->sendTimeout_ * 1000);
      $this->sendTimeoutSet_ = true;
    }

    while (Str\length($buf) > 0) {
      $buflen = Str\length($buf);
      $t_start = PHP\microtime(true);/* BEGIN_STRIP */
      WallTimeProfiler::pushInstance();
      $timer = WallTimeOperation::begin();
      WallTimeProfiler::get()->beginIOWait();
      /* END_STRIP */
      $got = (
        ()[zoned_local] ==>
          PHPism_FIXME::fwrite($this->handle_ as nonnull, $buf)
      )();
      $write_time = PHP\microtime(true) - $t_start;/* BEGIN_STRIP */

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

      if ($got === 0 || !($got is int)) {
        $read_err_detail = Str\format(
          '%d bytes from %s:%d to localhost:%d. Spent %2.2f ms.',
          $buflen,
          $this->host_,
          $this->port_,
          $this->lport_,
          HH\FIXME\UNSAFE_CAST<mixed, float>(
            $write_time * 1000,
            'Exposed by typing PHP\microtime',
          ),
        );
        $md = $this->getMetaData();
        if ($md['timed_out']) {
          throw new TTransportException(
            'TSocket: timeout while writing '.$read_err_detail,
            TTransportException::TIMED_OUT,
          );
        } else {
          throw new TTransportException(
            'TSocket: could not write '.$read_err_detail,
            TTransportException::COULD_NOT_WRITE,
          );
        }
      }
      $buf = Str\slice($buf, $got);
    }

    $this->writeAttemptStart_ = null;
  }

  /**
   * Flush output to the socket.
   */
  <<__Override>>
  public function flush()[zoned_shallow]: void {/* BEGIN_STRIP */
    WallTimeProfiler::pushInstance();
    $timer = WallTimeOperation::begin();
    WallTimeProfiler::get()->beginIOWait();
    /* END_STRIP */
    $ret = call_defaults_from_zoned_shallow(
      ()[defaults] ==> PHP\fflush($this->handle_ as nonnull),
      'Blocked by a migration of builtins_file to coeffects (T107309662).',
    );
    /* BEGIN_STRIP */
    WallTimeProfiler::get()->endIOWait(null);
    $timer->end();
    WallTimeProfiler::popInstance();

    $this->incrCount(ProfilingCounterCount::THRIFT_FLUSH_COUNT);
    $this->incrDuration(
      ProfilingCounterDuration::THRIFT_FLUSH_DURATION,
      $timer->getOldestRunningIODuration(),
    );
    /* END_STRIP */

    if ($ret === false) {
      throw new TTransportException(
        'TSocket: could not flush '.$this->host_.':'.$this->port_,
      );
    }
  }/* BEGIN_STRIP */

  /**
   * Name of the transport (e.g.: socket).
   */
  <<__Override>>
  public function getTransportType()[]: string {
    return 'socket';
  }
  /* END_STRIP */
}
