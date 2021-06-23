<?hh
/*
 *  Copyright (c) 2017-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the hphp/hsl/ subdirectory of this source tree.
 *
 */

namespace HH\Lib\IO;

use namespace HH\Lib\{Math, OS, Str};
use namespace HH\Lib\_Private\{_IO, _OS};

enum MemoryHandleWriteMode: int {
  OVERWRITE = 0;
  APPEND = OS\O_APPEND;
}

/** Read from/write to an in-memory buffer.
 *
 * This class is intended for use in unit tests.
 *
 * @see `IO\pipe()` for more complicated tests
 */
final class MemoryHandle implements CloseableSeekableReadWriteHandle {
  use ReadHandleConvenienceMethodsTrait;
  use WriteHandleConvenienceMethodsTrait;

  private int $offset = 0;
  private bool $open = true;

  public function __construct(
    private string $buffer = '',
    private MemoryHandleWriteMode $writeMode = MemoryHandleWriteMode::OVERWRITE,
  ) {
  }

  public function close(): void {
    $this->open = false;
    $this->offset = -1;
  }

  <<__ReturnDisposable>>
  public function closeWhenDisposed(): \IDisposable {
    return new _IO\CloseWhenDisposed($this);
  }

  public async function readAllowPartialSuccessAsync(
    ?int $max_bytes = null,
    ?int $_timeout_nanos = null,
  ): Awaitable<string> {
    return $this->readImpl($max_bytes);
  }

  public function readImpl(?int $max_bytes = null): string {
    $this->checkIsOpen();

    $max_bytes ??= Math\INT64_MAX;
    _OS\arg_assert($max_bytes > 0, '$max_bytes must be null or positive');
    $len = Str\length($this->buffer);
    if ($this->offset >= $len) {
      return '';
    }
    $to_read = Math\minva($max_bytes, $len - $this->offset);

    $ret = Str\slice($this->buffer, $this->offset, $to_read);
    $this->offset += $to_read;
    return $ret;
  }

  public function seek(int $pos): void {
    $this->checkIsOpen();

    _OS\arg_assert($pos >= 0, "Position must be >= 0");
    // Past end of file is explicitly fine
    $this->offset = $pos;
  }

  public function tell(): int {
    $this->checkIsOpen();
    return $this->offset;
  }

  protected function writeImpl(string $data): int {
    $this->checkIsOpen();
    $length = Str\length($this->buffer);
    if ($length < $this->offset) {
      $this->buffer .= Str\repeat("\0", $this->offset - $length);
      $length = $this->offset;
    }

    if ($this->writeMode === MemoryHandleWriteMode::APPEND) {
      $this->buffer .= $data;
      $this->offset = Str\length($this->buffer);
      return Str\length($data);
    }

    _OS\arg_assert(
      $this->writeMode === MemoryHandleWriteMode::OVERWRITE,
      "Write mode must be OVERWRITE or APPEND",
    );

    $data_length = Str\length($data);
    $new = Str\slice($this->buffer, 0, $this->offset).$data;
    if ($this->offset < $length) {
      $new .= Str\slice(
        $this->buffer,
        Math\minva($this->offset + $data_length, $length),
      );
    }
    $this->buffer = $new;
    $this->offset += $data_length;
    return $data_length;
  }

  public async function writeAllowPartialSuccessAsync(
    string $data,
    ?int $timeout_nanos = null,
  ): Awaitable<int> {
    return $this->writeImpl($data);
  }

  public function getBuffer(): string {
    return $this->buffer;
  }

  /** Set the internal buffer and reset position to the beginning of the file.
   *
   * If you wish to preserve the position, use `tell()` and `seek()`,
   * or `appendToBuffer()`.
   */
  public function reset(string $data = ''): void {
    $this->open = true;
    $this->buffer = $data;
    $this->offset = 0;
  }

  /** Append data to the internal buffer, preserving position.
   *
   * @see `write()` if you want the offset to be changed.
   */
  public function appendToBuffer(string $data): void {
    $this->checkIsOpen();
    $this->buffer .= $data;
  }

  private function checkIsOpen(): void {
    if (!$this->open) {
      _OS\throw_errno(
        OS\Errno::EBADF,
        "%s::close() was already called",
        self::class,
      );
    }
  }
}
