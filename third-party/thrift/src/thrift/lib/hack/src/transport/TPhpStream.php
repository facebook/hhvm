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
 * Php stream transport. Reads to and writes from the php standard streams
 * php://input and php://output
 *
 * @package thrift.transport
 */
<<Oncalls('thrift')>> // @oss-disable
final class TPhpStream extends TTransport {

  const int MODE_R = 1;
  const int MODE_W = 2;

  private ?resource $inStream_ = null;

  private ?resource $outStream_ = null;

  private bool $read_ = false;

  private bool $write_ = false;

  /**
   * Specifies the maximum number of bytes to read
   * at once from internal stream.
   */
  private ?int $maxReadChunkSize_ = null;

  public function __construct(int $mode)[] {
    $this->read_ = (bool)($mode & self::MODE_R);
    $this->write_ = (bool)($mode & self::MODE_W);
  }

  <<__Override>>
  public function open()[leak_safe]: void {/* BEGIN_STRIP */
    WallTimeProfiler::pushInstance();
    $timer = WallTimeOperation::begin();
    WallTimeProfiler::get()->beginIOWait();
    try {
      /* END_STRIP */
      if ($this->read_) {
        $this->inStream_ = HH\FIXME\UNSAFE_CAST<mixed, ?resource>(
          HH\Coeffects\fb\backdoor_from_leak_safe__DO_NOT_USE(
            ()[defaults] ==> PHPism_FIXME::fopen(self::inStreamName(), 'r'),
            'Blocked by a migration of builtins_file to coeffects (T107309662).',
          ),
          'FIXME[4110] fopen lies about its type',
        );
        if (!($this->inStream_ is resource)) {
          throw new TException('TPhpStream: Could not open php://input');
        }
      }
      if ($this->write_) {
        $this->outStream_ = HH\FIXME\UNSAFE_CAST<mixed, ?resource>(
          HH\Coeffects\fb\backdoor_from_leak_safe__DO_NOT_USE(
            ()[defaults] ==> PHPism_FIXME::fopen('php://output', 'w'),
            'Blocked by a migration of builtins_file to coeffects (T107309662).',
          ),
          'FIXME[4110] fopen lies about its type',
        );
        if (!($this->outStream_ is resource)) {
          throw new TException('TPhpStream: Could not open php://output');
        }
      }/* BEGIN_STRIP */
    } finally {
      WallTimeProfiler::get()->endIOWait(null);
      $timer->end();
      WallTimeProfiler::popInstance();

      $this->incrCount(ProfilingCounterCount::THRIFT_OPEN_COUNT);
      $this->incrDuration(
        ProfilingCounterDuration::THRIFT_OPEN_DURATION,
        $timer->getOldestRunningIODuration(),
      );
    }
    /* END_STRIP */
  }

  <<__Override>>
  public function close()[leak_safe]: void {
    if ($this->read_) {
      HH\Coeffects\fb\backdoor_from_leak_safe__DO_NOT_USE(
        ()[defaults] ==> PHPism_FIXME::fclose($this->inStream_ as nonnull),
        'Blocked by a migration of builtins_file to coeffects (T107309662).',
      );
      $this->inStream_ = null;
    }
    if ($this->write_) {
      HH\Coeffects\fb\backdoor_from_leak_safe__DO_NOT_USE(
        ()[defaults] ==> PHPism_FIXME::fclose($this->outStream_ as nonnull),
        'Blocked by a migration of builtins_file to coeffects (T107309662).',
      );
      $this->outStream_ = null;
    }
  }

  <<__Override>>
  public function isOpen()[]: bool {
    return (!$this->read_ || $this->inStream_ is resource) &&
      (!$this->write_ || $this->outStream_ is resource);
  }

  <<__Override>>
  public function read(int $len)[zoned_shallow]: string {
    if ($this->maxReadChunkSize_ !== null) {
      $len = Math\minva($len, $this->maxReadChunkSize_);
    }/* BEGIN_STRIP */

    WallTimeProfiler::pushInstance();
    $timer = WallTimeOperation::begin();
    WallTimeProfiler::get()->beginIOWait();
    /* END_STRIP */

    $data = call_defaults_from_zoned_shallow(
      ()[defaults] ==> PHPism_FIXME::fread($this->inStream_ as nonnull, $len),
      'Blocked by a migration of builtins_file to coeffects (T107309662).',
    );
    /* BEGIN_STRIP */

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
    if ($data === false || $data === '') {
      throw new TException('TPhpStream: Could not read '.$len.' bytes');
    }
    return $data;
  }

  <<__Override>>
  public function write(string $buf)[zoned_shallow]: void {
    while (Str\length($buf) > 0) {/* BEGIN_STRIP */
      WallTimeProfiler::pushInstance();
      $timer = WallTimeOperation::begin();
      WallTimeProfiler::get()->beginIOWait();
      /* END_STRIP */
      $got = call_defaults_from_zoned_shallow(
        ()[defaults] ==>
          PHPism_FIXME::fwrite($this->outStream_ as nonnull, $buf),
        'Blocked by a migration of builtins_file to coeffects (T107309662).',
      );
      /* BEGIN_STRIP */
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
        throw new TException(
          'TPhpStream: Could not write '.(string)Str\length($buf).' bytes',
        );
      }
      $buf = PHP\substr($buf, $got);
    }
  }

  <<__Override>>
  public function flush()[zoned_shallow]: void {/* BEGIN_STRIP */
    WallTimeProfiler::pushInstance();
    $timer = WallTimeOperation::begin();
    WallTimeProfiler::get()->beginIOWait();
    PHPism_FIXME::suppressAllErrors(
      ()[zoned_shallow] ==> call_defaults_from_zoned_shallow(
        ()[defaults] ==> PHP\fflush($this->outStream_ as nonnull),
        'Blocked by a migration of builtins_file to coeffects (T107309662).',
      ),
    );/* BEGIN_STRIP */
    WallTimeProfiler::get()->endIOWait(null);
    $timer->end();
    WallTimeProfiler::popInstance();

    $this->incrCount(ProfilingCounterCount::THRIFT_FLUSH_COUNT);
    $this->incrDuration(
      ProfilingCounterDuration::THRIFT_FLUSH_DURATION,
      $timer->getOldestRunningIODuration(),
    );
    /* END_STRIP */
  }

  private static function inStreamName()[read_globals]: string {
    $php_sapi_name = PHP\php_sapi_name();
    if ($php_sapi_name === 'cli') {
      return 'php://stdin';
    }
    return 'php://input';
  }/* BEGIN_STRIP */

  /**
   * Name of the transport (e.g.: socket).
   */
  <<__Override>>
  public function getTransportType()[]: string {
    return 'stream';
  }
  /* END_STRIP */
}
