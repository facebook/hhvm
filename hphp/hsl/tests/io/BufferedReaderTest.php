<?hh // strict
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the hphp/hsl/ subdirectory of this source tree.
 *
 */

use namespace HH\Lib\{File, IO, OS, Str, Vec};
use namespace HH\Lib\_Private\{_IO, _OS};

use function HH\__Private\MiniTest\expect;
use type HH\__Private\MiniTest\HackTest;
use type HH\__Private\MiniTest\DataProvider;

final class BufferedReaderTest extends HackTest {
  public async function testReadByte(): Awaitable<void> {
    $r = new IO\BufferedReader(new IO\MemoryHandle('abc'));
    $a = await $r->readByteAsync();
    $b = await $r->readByteAsync();
    $c = await $r->readByteAsync();
    expect(vec[$a, $b, $c])->toEqual(vec['a', 'b', 'c']);
    expect(async () ==> await $r->readByteAsync())->toThrow(
      OS\BrokenPipeException::class,
    );

    $r = new IO\BufferedReader(new IO\MemoryHandle('abcdef'));
    expect(await $r->readByteAsync())->toEqual('a');
    expect(await $r->readFixedSizeAsync(2))->toEqual('bc');
    expect(await $r->readAllowPartialSuccessAsync(2))->toEqual('de');
    expect(await $r->readByteAsync())->toEqual('f');

    $r = new IO\BufferedReader(new IO\MemoryHandle('abcdef'));
    expect(await $r->readByteAsync())->toEqual('a');
    expect(await $r->readAllAsync())->toEqual('bcdef');
  }

  public async function testReadFixedSize(): Awaitable<void> {
    $r = new IO\BufferedReader(new IO\MemoryHandle('abcdef'));
    $abc = await $r->readFixedSizeAsync(3);
    $def = await $r->readFixedSizeAsync(3);
    expect(vec[$abc, $def])->toEqual(vec['abc', 'def']);
    expect(async () ==> await $r->readFixedSizeAsync(3))->toThrow(
      OS\BrokenPipeException::class,
    );

    $r = new IO\BufferedReader(new IO\MemoryHandle('abc'));
    expect(async () ==> await $r->readFixedSizeAsync(6))->toThrow(
      OS\BrokenPipeException::class,
    );

    $r = new IO\BufferedReader(new IO\MemoryHandle('abcdef'));
    expect(await $r->readFixedSizeAsync(2))->toEqual('ab');
    expect(await $r->readFixedSizeAsync(2))->toEqual('cd');
    expect(await $r->readFixedSizeAsync(2))->toEqual('ef');
  }

  public async function testReadTooMuch(): Awaitable<void> {
    $newbuf = () ==> new IO\BufferedReader(new IO\MemoryHandle('abc'));
    expect(await $newbuf()->readAllAsync(6))->toEqual('abc');
    expect(await $newbuf()->readAllowPartialSuccessAsync(6))->toEqual('abc');
    expect(async () ==> await $newbuf()->readFixedSizeAsync(6))->toThrow(
      OS\BrokenPipeException::class,
    );
  }

  public async function testReadLine(): Awaitable<void> {
    $r = new IO\BufferedReader(new IO\MemoryHandle("ab\ncd\nef"));
    expect(await $r->readLineAsync())->toEqual("ab");
    expect(await $r->readLineAsync())->toEqual("cd");
    expect(await $r->readLineAsync())->toEqual("ef");
    expect(await $r->readLineAsync())->toBeNull();
    expect(async () ==> await $r->readLinexAsync())->toThrow(
      OS\BrokenPipeException::class,
    );

    $r = new IO\BufferedReader(new IO\MemoryHandle("ab\ncd\nef"));
    expect(await $r->readLineAsync())->toEqual("ab");
    expect(await $r->readLineAsync())->toEqual("cd");
    expect(await $r->readAllAsync())->toEqual('ef');

    $r = new IO\BufferedReader(new IO\MemoryHandle('ab'));
    expect(await $r->readLineAsync())->toEqual('ab');
    expect(await $r->readLineAsync())->toBeNull();
  }

  public async function testReadUntil(): Awaitable<void> {
    $r = new IO\BufferedReader(new IO\MemoryHandle("ab\r\ncd\r\n"));
    expect(await $r->readLineAsync())->toEqual("ab\r");
    expect(await $r->readLineAsync())->toEqual("cd\r");

    $r = new IO\BufferedReader(new IO\MemoryHandle("ab\r\ncd\r\n"));
    expect(await $r->readUntilAsync("\r\n"))->toEqual("ab");
    expect(await $r->readUntilAsync("\r\n"))->toEqual("cd");

    $r = new IO\BufferedReader(new IO\MemoryHandle("abFOOcdFOO"));
    expect(await $r->readUntilAsync("FOO"))->toEqual("ab");
    expect(await $r->readUntilAsync("FOO"))->toEqual("cd");

    // Start with readByteAsync so we have a non-empty buffer
    $r = new IO\BufferedReader(new IO\MemoryHandle("_abFOOcdFOO"));
    $_ = await $r->readByteAsync();
    expect(await $r->readUntilAsync("FOO"))->toEqual("ab");
    expect(await $r->readUntilAsync("FOO"))->toEqual("cd");
  }

  public async function testReadUntilBufferBoundary(): Awaitable<void> {
    // Intent is to test the case when the separator starts in one chunk, and
    // ends in another, i.e.:
    // - Str\length($padding) < chunk size
    // - Str\length($padding.$separator) > chunk size
    $padding = Str\repeat('a', _IO\DEFAULT_READ_BUFFER_SIZE - 1);
    $separator = 'bc';

    list($r, $w) = IO\pipe();
    concurrent {
      await async {
        await $w->writeAllAsync($padding.$separator.'junk');
        $w->close();
      };
      await async {
        $br = new IO\BufferedReader($r);
        expect(await $br->readUntilAsync($separator))->toEqual($padding);
        $r->close();
      };
    }
  }

  public async function testReadLineVsReadUntil(): Awaitable<void> {
    $r = new IO\BufferedReader(new IO\MemoryHandle("ab\ncd"));
    expect(await $r->readLineAsync())->toEqual('ab');
    expect(await $r->readLineAsync())->toEqual('cd');

    $r = new IO\BufferedReader(new IO\MemoryHandle("ab\ncd"));
    expect(await $r->readUntilAsync("\n"))->toEqual('ab');
    expect(await $r->readUntilAsync("\n"))->toBeNull();
  }

  public static function provideLines(): vec<(string, vec<string>)> {
    /* Some of these seem unintuive, but they match libc fgets() and Rust
     * `lines()`; they also seem to match what people actually expect in
     * practice
     *
     * - Hit EOL? Everything up to there is a line
     * - Hit EOF? If we have content, it's a new line, but if not, there's
     *   nothing.
     */
    return vec[
      tuple('', vec[]), // myprog < /dev/null
      tuple("\n", vec['']), // echo | ./myprog
      tuple('foo', vec['foo']),
      tuple("foo\n", vec['foo']), // echo foo | ./myprog
      tuple("foo\nbar", vec['foo', 'bar']),
      tuple("foo\nbar\n", vec['foo', 'bar']),
      tuple("foo\nbar\n\n", vec['foo', 'bar', '']),
    ];
  }

  <<DataProvider('provideLines')>>
  public async function testIterateLines(
    string $input,
    vec<string> $expected_lines,
  ): Awaitable<void> {
    $b = new IO\BufferedReader(new IO\MemoryHandle($input));
    $actual_lines = vec[];
    foreach ($b->linesIterator() await as $line) {
      $actual_lines[] = $line;
    }
    expect($actual_lines)->toEqual(
      $expected_lines,
      "Input %s",
      \var_export($input, true),
    );
  }

  public async function testIterateLinesOnClosedFile(): Awaitable<void> {
    list($r, $w) = IO\pipe();
    $r->close();
    $w->close();
    $b = new IO\BufferedReader($r);

    $lines = vec[];
    foreach ($b->linesIterator() await as $line) {
      $lines[] = $line;
    }
    expect($lines)->toEqual(vec[]);

  }

  public async function testEndOfFile(): Awaitable<void> {
    $b = new IO\BufferedReader(new IO\MemoryHandle(''));
    expect($b->isEndOfFile())->toBeTrue();

    // Closed, no data.
    list($r, $w) = IO\pipe();
    $r->close();
    $w->close();
    $b = new IO\BufferedReader($r);
    expect($b->isEndOfFile())->toBeTrue();

    $b = new IO\BufferedReader(new IO\MemoryHandle("foo\nbar\n"));
    expect($b->isEndOfFile())->toBeFalse();
    expect(await $b->readLineAsync())->toEqual("foo");
    expect($b->isEndOfFile())->toBeFalse();
    expect(await $b->readLineAsync())->toEqual("bar");

    expect(async () ==> await $b->readLinexAsync())->toThrow(OS\BrokenPipeException::class);
    expect($b->isEndOfFile())->toBeTrue();
  }

  public async function testReadByteAfterBufferSize(): Awaitable<void> {
    using $tf = File\temporary_file();
    $in = Str\repeat('a', _IO\DEFAULT_READ_BUFFER_SIZE).'b';
    $f = $tf->getHandle();
    await $f->writeAllAsync($in);
    $f->seek(0);
    $br = new IO\BufferedReader($f);
    $out = '';
    while (!$br->isEndOfFile()) {
      $byte = await $br->readByteAsync();
      expect(Str\length($byte))->toEqual(1);
      $out .= $byte;
    }
    expect(Str\ends_with($out, 'b'))->toBeTrue('Missing final byte');
    expect(Str\length($out))->toEqual(Str\length($in));
    expect($out)->toEqual($in);
  }
}
