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

final class TFileStreamBuffer {
  private string $buffer = '';
  private int $bufferIndex = 0;

  public function __construct(private int $bufferSize)[] {}

  public function setBufferSize(int $buffer_size)[write_props]: this {
    $this->bufferSize = $buffer_size;
    return $this;
  }

  public function getAvailableBytes()[]: int {
    return Str\length($this->buffer) - $this->bufferIndex;
  }

  public function append(string $data)[write_props]: void {
    $this->buffer .= $data;
  }

  public function detach(?int $len = null)[write_props]: string {
    if ($len === null) {
      $result = Str\slice($this->buffer, $this->bufferIndex);
      $this->buffer = '';
      $this->bufferIndex = 0;
      return $result;
    }

    if ($len === 1) {
      $result = $this->buffer[$this->bufferIndex];
      $this->bufferIndex++;
    } else {
      $result = $this->peek($len);
      $this->bufferIndex += $len;
    }

    // If we read enough of the buffer throw away the stuff that we read so we
    // don't use too much memory
    if ($this->bufferIndex >= $this->bufferSize) {
      $this->buffer = Str\slice($this->buffer, $this->bufferIndex);
      $this->bufferIndex = 0;
    }

    return $result;
  }

  public function peek(?int $len)[]: string {
    if ($len === 1) {
      return $this->buffer[$this->bufferIndex];
    }
    return Str\slice($this->buffer, $this->bufferIndex, $len);
  }
}

enum TFileStreamMode: int {
  MODE_R = 1;
  MODE_W = 2;
}

/**
 * File stream transport. Reads to and writes from the file streams
 *
 * @package thrift.transport
 */
<<Oncalls('thrift')>> // @oss-disable
final class TFileStream extends TTransport implements IThriftBufferedTransport {

  private ?resource $inStream = null;
  private ?resource $outStream = null;

  private int $bufferSize = 1024 * 1024;
  <<TestsBypassVisibility>> private TFileStreamBuffer $buffer;

  public function __construct(
    private TFileStreamMode $mode,
    private string $filePath,
  )[] {
    $this->buffer = new TFileStreamBuffer($this->bufferSize);
  }

  public function setBufferSize(int $buffer_size)[write_props]: this {
    $this->bufferSize = $buffer_size;
    $this->buffer->setBufferSize($buffer_size);
    return $this;
  }

  <<__Override>>
  public function open()[leak_safe]: void {
    switch ($this->mode) {
      case TFileStreamMode::MODE_R:
        $this->inStream = HH\Coeffects\fb\backdoor_from_leak_safe__DO_NOT_USE(
          ()[defaults] ==> PHP\fopen($this->filePath, 'r'),
          'Blocked by a migration of builtins_file to coeffects (T107309662).',
        );
        if (!($this->inStream is resource)) {
          throw
            new TException('TPhpStream: Could not open %s', $this->filePath);
        }
        break;
      case TFileStreamMode::MODE_W:
        $this->outStream = HH\Coeffects\fb\backdoor_from_leak_safe__DO_NOT_USE(
          ()[defaults] ==> PHP\fopen($this->filePath, 'w'),
          'Blocked by a migration of builtins_file to coeffects (T107309662).',
        );
        if (!($this->outStream is resource)) {
          throw
            new TException('TPhpStream: Could not open %s', $this->filePath);
        }
        break;
    }
  }

  <<__Override>>
  public function close()[zoned_shallow]: void {
    $this->flush();
    switch ($this->mode) {
      case TFileStreamMode::MODE_R:
        call_defaults_from_zoned_shallow(
          ()[defaults] ==> PHP\fclose($this->inStream as nonnull),
          'Blocked by a migration of builtins_file to coeffects (T107309662).',
        );
        $this->inStream = null;
        break;
      case TFileStreamMode::MODE_W:
        $this->flush();
        call_defaults_from_zoned_shallow(
          ()[defaults] ==> PHP\fclose($this->outStream as nonnull),
          'Blocked by a migration of builtins_file to coeffects (T107309662).',
        );
        $this->outStream = null;
        break;
    }
  }

  <<__Override>>
  public function isOpen()[]: bool {
    switch ($this->mode) {
      case TFileStreamMode::MODE_R:
        return $this->inStream is resource;
      case TFileStreamMode::MODE_W:
        return $this->outStream is resource;
    }
  }

  /**
   * Reads maximum $len bytes from the stream.
   */
  <<__Override>>
  public function read(int $len)[zoned_shallow]: string {
    $available_bytes = $this->buffer->getAvailableBytes();
    if ($available_bytes < $len) {
      $this->fill($len - $available_bytes + $this->bufferSize);
    }
    return $this->buffer
      ->detach(Math\minva($len, $this->buffer->getAvailableBytes()));
  }

  /**
   * Peek some bytes in the buffer without removing the bytes from it
   *
   * @param int $len   length to peek
   * @param int $start the start position of the returned string
   */
  public function peek(int $len, int $start = 0)[zoned_shallow]: string {
    $bytes_needed = $len + $start;

    $available_bytes = $this->buffer->getAvailableBytes();
    if ($available_bytes < $bytes_needed) {
      $this->fill($bytes_needed - $available_bytes + $this->bufferSize);
    }
    return $this->buffer
      ->peek(Math\minva($len + $start, $this->buffer->getAvailableBytes()))
      |> Str\slice($$, $start);
  }

  private function fill(int $amount)[zoned_shallow]: void {
    $data = call_defaults_from_zoned_shallow(
      ()[defaults] ==> PHPism_FIXME::fread($this->inStream as nonnull, $amount),
      'Blocked by a migration of builtins_file to coeffects (T107309662).',
    );
    if ($data === false) {
      throw new TException(
        Str\format('TFileStream: Could not read %d bytes', $amount),
      );
    }
    $this->buffer->append($data);
  }

  <<__Override>>
  public function write(string $data)[zoned_shallow]: void {
    if (
      $this->buffer->getAvailableBytes() + Str\length($data) > $this->bufferSize
    ) {
      $to_write = $this->buffer->detach();
      $written = call_defaults_from_zoned_shallow(
        ()[defaults] ==>
          PHPism_FIXME::fwrite($this->outStream as nonnull, $to_write),
        'Blocked by a migration of builtins_file to coeffects (T107309662).',
      );
      if ($written === 0 || $written === false) {
        throw new TException(
          'TPhpStream: Could not write %d bytes in buffer',
          Str\length($to_write),
        );
      }
      $written = call_defaults_from_zoned_shallow(
        ()[defaults] ==>
          PHPism_FIXME::fwrite($this->outStream as nonnull, $data),
        'Blocked by a migration of builtins_file to coeffects (T107309662).',
      );
      if ($written === 0 || $written === false) {
        throw new TException(
          'TPhpStream: Could not write %d bytes in buffer',
          Str\length($data),
        );
      }
    } else {
      $this->buffer->append($data);
    }
  }

  <<__Override>>
  public function flush()[zoned_shallow]: void {
    switch ($this->mode) {
      case TFileStreamMode::MODE_R:
        break;
      case TFileStreamMode::MODE_W:
        if ($this->buffer->getAvailableBytes() > 0) {
          $to_write = $this->buffer->detach();
          $written = call_defaults_from_zoned_shallow(
            ()[defaults] ==>
              PHPism_FIXME::fwrite($this->outStream as nonnull, $to_write),
            'Blocked by a migration of builtins_file to coeffects (T107309662).',
          );
          if ($written === 0 || $written === false) {
            throw new TException(
              'TPhpStream: Could not write %d bytes in buffer',
              Str\length($to_write),
            );
          }
        }
        call_defaults_from_zoned_shallow(
          ()[defaults] ==> PHP\fflush($this->outStream as nonnull),
          'Blocked by a migration of builtins_file to coeffects (T107309662).',
        );
        break;
    }
  }

  /**
   * Name of the transport (e.g.: socket).
   */
  <<__Override>>
  public function getTransportType()[]: string {
    return 'filestream';
  }
}
