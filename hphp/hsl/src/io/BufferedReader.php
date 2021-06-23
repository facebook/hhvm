<?hh
/*
 *  Copyright (c) 2017-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the hphp/hsl/ subdirectory of this source tree.
 *
 */

/* @lint-ignore-every AWAIT_IN_LOOP */

namespace HH\Lib\IO;

use namespace HH\Lib\{IO, Math, OS, Str};
use namespace HH\Lib\_Private\_OS;

/** Wrapper for `ReadHandle`s, with buffered line-based byte-based accessors.
 *
 * - `readLineAsync()` is similar to `fgets()`
 * - `readUntilAsync()` is a more general form
 * - `readByteAsync()` is similar to `fgetc()`
 */
final class BufferedReader implements IO\ReadHandle {
  use ReadHandleConvenienceMethodsTrait;

  public function __construct(private IO\ReadHandle $handle) {
  }

  public function getHandle(): IO\ReadHandle {
    return $this->handle;
  }

  private bool $eof = false;
  private string $buffer = '';

  // implementing interface
  public function readImpl(?int $max_bytes = null): string {
    _OS\arg_assert(
      $max_bytes is null || $max_bytes > 0,
      "Max bytes must be null, or greater than 0",
    );

    if ($this->eof) {
      return '';
    }
    if ($this->buffer === '') {
      $this->buffer = $this->getHandle()->readImpl();
      if ($this->buffer === '') {
        $this->eof = true;
        return '';
      }
    }

    if ($max_bytes is null || $max_bytes >= Str\length($this->buffer)) {
      $buf = $this->buffer;
      $this->buffer = '';
      return $buf;
    }
    $buf = $this->buffer;
    $this->buffer = Str\slice($buf, $max_bytes);
    return Str\slice($buf, 0, $max_bytes);
  }

  public async function readAllowPartialSuccessAsync(
    ?int $max_bytes = null,
    ?int $timeout_ns = null,
  ): Awaitable<string> {
    _OS\arg_assert(
      $max_bytes is null || $max_bytes > 0,
      "Max bytes must be null, or greater than 0",
    );
    _OS\arg_assert(
      $timeout_ns is null || $timeout_ns > 0,
      "Timeout must be null, or greater than 0",
    );

    if ($this->eof) {
      return '';
    }
    if ($this->buffer === '') {
      await $this->fillBufferAsync(null, $timeout_ns);
    }

    // We either have a buffer, or reached EOF; either way, behavior matches
    // read, so just delegate
    return $this->readImpl($max_bytes);
  }

  /** Read until the specified suffix is seen.
   *
   * The trailing suffix is read (so won't be returned by other calls), but is not
   * included in the return value.
   *
   * This call returns null if the suffix is not seen, even if there is other
   * data.
   *
   * @see `readUntilxAsync()` if you want to throw EPIPE instead of returning null
   * @see `linesIterator()` if you want to iterate over all lines
   * @see `readLineAsync()` if you want trailing data instead of null
   */
  public async function readUntilAsync(string $suffix): Awaitable<?string> {
    $buf = $this->buffer;
    $idx = Str\search($buf, $suffix);
    $suffix_len = Str\length($suffix);
    if ($idx !== null) {
      $this->buffer = Str\slice($buf, $idx + $suffix_len);
      return Str\slice($buf, 0, $idx);
    }

    do {
      // + 1 as it would have been matched in the previous iteration if it
      // fully fit in the chunk
      $offset = Math\maxva(0, Str\length($buf) - $suffix_len + 1);
      $chunk = await $this->handle->readAllowPartialSuccessAsync();
      if ($chunk === '') {
        $this->buffer = $buf;
        return null;
      }
      $buf .= $chunk;
      $idx = Str\search($buf, $suffix, $offset);
    } while ($idx === null);

    $this->buffer = Str\slice($buf, $idx + $suffix_len);
    return Str\slice($buf, 0, $idx);
  }

  /** Read until the suffix, or raise EPIPE if the separator is not seen.
   *
   * This is similar to `readUntilAsync()`, however it raises EPIPE instead
   * of returning null.
   */
  public async function readUntilxAsync(string $suffix): Awaitable<string> {
    $ret = await $this->readUntilAsync($suffix);
    if ($ret === null) {
      throw new OS\BrokenPipeException(
        OS\Errno::EPIPE,
        'Marker/suffix not found before end of file',
      );
    }
    return $ret;
  }

  /** Read until the platform end-of-line sequence is seen, or EOF is reached.
   *
   * On current platforms, this is always `\n`; it may have other values on other
   * platforms in the future, e.g. `\r\n`.
   *
   * The newline sequence is read (so won't be returned by other calls), but is not
   * included in the return value.
   *
   * - Returns null if the end of file is reached with no data.
   * - Returns a string otherwise
   *
   * Some illustrative edge cases:
   * - `''` is considered a 0-line input
   * - `'foo'` is considered a 1-line input
   * - `"foo\nbar"` is considered a 2-line input
   * - `"foo\nbar\n"` is also considered a 2-line input
   *
   * @see `linesIterator()` for an iterator
   * @see `readLinexAsync()` to throw EPIPE instead of returning null
   * @see `readUntilAsync()` for a more general form
   */
  public async function readLineAsync(): Awaitable<?string> {
    try {
      $line = await $this->readUntilAsync("\n");
    } catch (OS\ErrnoException $ex) {
      if ($ex->getErrno() === OS\Errno::EBADF) {
        // Eg foreach ($stdin->linesIterator()) when stdin is closed
        return null;
      }
      throw $ex;
    }

    if ($line !== null) {
      return $line;
    }

    $line = await $this->readAllAsync();
    return $line === '' ? null : $line;
  }

  /** Read a line or throw EPIPE.
   *
   * @see `readLineAsync()` for details.
   */
  public async function readLinexAsync(): Awaitable<string> {
    $line = await $this->readLineAsync();
    if ($line !== null) {
      return $line;
    }
    throw new OS\BrokenPipeException(OS\Errno::EPIPE, 'No more lines to read.');
  }

  /** Iterate over all lines in the file.
   *
   * Usage:
   *
   * ```
   * foreach ($reader->linesIterator() await as $line) {
   *   do_stuff($line);
   * }
   * ```
   */
  public function linesIterator(): AsyncIterator<string> {
    return new BufferedReaderLineIterator($this);
  }

  <<__Override>> // from trait
  public async function readFixedSizeAsync(
    int $size,
    ?int $timeout_ns = null,
  ): Awaitable<string> {
    $timer = new \HH\Lib\_Private\OptionalIncrementalTimeout(
      $timeout_ns,
      () ==> {
        _OS\throw_errno(
          OS\Errno::ETIMEDOUT,
          "Reached timeout before reading requested amount of data",
        );
      },
    );
    while (Str\length($this->buffer) < $size && !$this->eof) {
      await $this->fillBufferAsync(
        $size - Str\length($this->buffer),
        $timer->getRemainingNS(),
      );
    }
    if ($this->eof) {
      throw new OS\BrokenPipeException(
        OS\Errno::EPIPE,
        'Reached end of file before requested size',
      );
    }
    $buffer_size = Str\length($this->buffer);
    invariant(
      $buffer_size >= $size,
      "Should have read the requested data or reached EOF",
    );
    if ($size === $buffer_size) {
      $ret = $this->buffer;
      $this->buffer = '';
      return $ret;
    }
    $ret = Str\slice($this->buffer, 0, $size);
    $this->buffer = Str\slice($this->buffer, $size);
    return $ret;
  }

  /** Read a single byte from the handle.
   *
   * Fails with EPIPE if the handle is closed or otherwise unreadable.
   */
  public async function readByteAsync(
    ?int $timeout_ns = null,
  ): Awaitable<string> {
    _OS\arg_assert(
      $timeout_ns is null || $timeout_ns > 0,
      "Timeout must be null, or greater than 0",
    );
    if ($this->buffer === '' && !$this->eof) {
      await $this->fillBufferAsync(null, $timeout_ns);
    }
    if ($this->buffer === '') {
      _OS\throw_errno(OS\Errno::EPIPE, "Reached EOF without any more data");
    }
    $ret = $this->buffer[0];
    if ($ret === $this->buffer) {
      $this->buffer = '';
      $this->eof = true;
      return $ret;
    }
    $this->buffer = Str\slice($this->buffer, 1);
    return $ret;
  }

  /** If we are known to have reached the end of the file.
   *
   * This function is best-effort: `true` is reliable, but `false` is more of
   * 'maybe'. For example, if called on an open socket with no data available,
   * it will return `false`; it is then possible that a future read will:
   * - return data if the other send sends some more
   * - block forever, or until timeout if set
   * - return the empty string if the socket closes the connection
   *
   * Additionally, helpers such as `readUntil` may fail with `EPIPE`.
   */
  public function isEndOfFile(): bool {
    if ($this->eof) {
      return true;
    }
    if ($this->buffer !== '') {
      return false;
    }

    // attempt to make `while (!$handle->isEOF()) {` safe on a closed file
    // handle, e.g. STDIN; if we just return `$this->eof`, the caller loop
    // body must check for EPIPE and EBADF which is unexpected.
    try {
      // Calling the non-async (but still non-blocking) version as the async
      // version could wait for the other end to send data - which could lead
      // to both ends of a pipe/socket waiting on each other.
      $this->buffer = $this->handle->readImpl();
      if ($this->buffer === '') {
        $this->eof = true;
        return true;
      }
    } catch (OS\BlockingIOException $_EWOULDBLOCK) {
      return false;
    } catch (OS\ErrnoException $ex) {
      if ($ex->getErrno() === OS\Errno::EBADF) {
        $this->eof = true;
        return true;
      }
      // ignore; they'll hit it again when they try a real read
    }
    return false;
  }

  private async function fillBufferAsync(
    ?int $desired_bytes,
    ?int $timeout_ns,
  ): Awaitable<void> {
    $chunk = await $this->getHandle()
      ->readAllowPartialSuccessAsync($desired_bytes, $timeout_ns);
    if ($chunk === '') {
      $this->eof = true;
    }
    $this->buffer .= $chunk;
  }
}
