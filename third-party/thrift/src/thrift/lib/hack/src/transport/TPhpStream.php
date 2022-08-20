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
 * Php stream transport. Reads to and writes from the php standard streams
 * php://input and php://output
 *
 * @package thrift.transport
 */
class TPhpStream extends TTransport {

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

  public function __construct(int $mode) {
    $this->read_ = (bool) ($mode & self::MODE_R);
    $this->write_ = (bool) ($mode & self::MODE_W);
  }

  /**
   * Sets the internal max read chunk size.
   * null for no limit (default).
   */
  public function setMaxReadChunkSize(int $maxReadChunkSize): void {
    $this->maxReadChunkSize_ = $maxReadChunkSize;
  }

  public function open(): void {
    if ($this->read_) {
      $this->inStream_ = @fopen(self::inStreamName(), 'r');
      if (!is_resource($this->inStream_)) {
        throw new TException('TPhpStream: Could not open php://input');
      }
    }
    if ($this->write_) {
      $this->outStream_ = @fopen('php://output', 'w');
      if (!is_resource($this->outStream_)) {
        throw new TException('TPhpStream: Could not open php://output');
      }
    }
  }

  public function close(): void {
    if ($this->read_) {
      @fclose($this->inStream_);
      $this->inStream_ = null;
    }
    if ($this->write_) {
      @fclose($this->outStream_);
      $this->outStream_ = null;
    }
  }

  public function isOpen(): bool {
    return
      (!$this->read_ || is_resource($this->inStream_)) &&
      (!$this->write_ || is_resource($this->outStream_));
  }

  public function read(int $len): string {
    if ($this->maxReadChunkSize_ !== null) {
      $len = min($len, $this->maxReadChunkSize_);
    }

    $data = @fread($this->inStream_, $len);

    if ($data === false || $data === '') {
      throw new TException('TPhpStream: Could not read '.$len.' bytes');
    }
    return $data;
  }

  public function write(string $buf): void {
    while (strlen($buf) > 0) {
      $got = @fwrite($this->outStream_, $buf);

      if ($got === 0 || $got === false) {
        throw new TException(
          'TPhpStream: Could not write '.(string) strlen($buf).' bytes',
        );
      }
      $buf = substr($buf, $got);
    }
  }

  public function flush(): void {
    @fflush($this->outStream_);
  }

  private static function inStreamName(): string {
    if (php_sapi_name() == 'cli') {
      return 'php://stdin';
    }
    return 'php://input';
  }
}
