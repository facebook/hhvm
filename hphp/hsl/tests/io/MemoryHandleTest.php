<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the hphp/hsl/ subdirectory of this source tree.
 *
 */

use namespace HH\Lib\{IO, OS, Str};

use function HH\__Private\MiniTest\expect;
use type HH\__Private\MiniTest\HackTest;

final class MemoryHandleTest extends HackTest {
  public async function testWriteOverwriteOutOfBound(): Awaitable<void> {
    $handle = new IO\MemoryHandle('f', IO\MemoryHandleWriteMode::OVERWRITE);

    await $handle->writeAllAsync('Hello, World!');

    expect($handle->getBuffer())->toEqual('Hello, World!');
  }

  public async function testWriteAppendOutOfBound(): Awaitable<void> {
    $handle = new IO\MemoryHandle('f', IO\MemoryHandleWriteMode::APPEND);

    await $handle->writeAllAsync('Hello, World!');

    expect($handle->getBuffer())->toEqual('fHello, World!');
  }

  public async function testRead(): Awaitable<void> {
    $h = new IO\MemoryHandle('herpderp');
    expect(await $h->readFixedSizeAsync(4))->toEqual('herp');
    expect(await $h->readAllAsync())->toEqual('derp');
    expect(await $h->readAllAsync())->toEqual('');
    expect($h->tell())->toEqual(8);
    $h->seek(0);
    expect($h->tell())->toEqual(0);
    expect(await $h->readAllAsync())->toEqual('herpderp');
    $h->seek(4);
    expect($h->tell())->toEqual(4);
    expect(await $h->readAllAsync())->toEqual('derp');
  }

  public async function testCloseWhenDisposed(): Awaitable<void> {
    $h = new IO\MemoryHandle('foobar');
    using ($h->closeWhenDisposed()) {
      expect(await $h->readFixedSizeAsync(3))->toEqual('foo');
    }
    $ex = expect(async () ==> await $h->readFixedSizeAsync(3))->toThrow(
      OS\ErrnoException::class,
    );
    expect($ex->getErrno())->toEqual(OS\Errno::EBADF);
  }

  public async function testReadAtInvalidOffset(): Awaitable<void> {
    $h = new IO\MemoryHandle('herpderp');
    $h->seek(99999);
    expect(await $h->readAllAsync())->toEqual('');
  }

  public async function testReadTooMuch(): Awaitable<void> {
    $h = new IO\MemoryHandle("herpderp");
    expect(async () ==> await $h->readFixedSizeAsync(1024))->toThrow(
      OS\BrokenPipeException::class,
    );
  }

  public async function testWrite(): Awaitable<void> {
    $h = new IO\MemoryHandle();
    await $h->writeAllowPartialSuccessAsync('herp');
    expect($h->getBuffer())->toEqual('herp');
    await $h->writeAllowPartialSuccessAsync('derp');
    $h->reset();
    await $h->writeAllowPartialSuccessAsync('foo');
    expect($h->getBuffer())->toEqual('foo');
  }

  public async function testOverwrite(): Awaitable<void> {
    $h = new IO\MemoryHandle('xxxxderp');
    await $h->writeAllowPartialSuccessAsync('herp');
    expect($h->getBuffer())->toEqual('herpderp');
    expect(await $h->readAllAsync())->toEqual('derp');
    $h->seek(0);
    expect(await $h->readAllAsync())->toEqual('herpderp');
  }

  public async function testAppend(): Awaitable<void> {
    $h = new IO\MemoryHandle('herp', IO\MemoryHandleWriteMode::APPEND);
    await $h->writeAllowPartialSuccessAsync('derp');
    expect($h->getBuffer())->toEqual('herpderp');
    expect(await $h->readAllAsync())->toEqual('');
    $h->seek(0);
    expect(await $h->readAllAsync())->toEqual('herpderp');
  }

  public async function testReset(): Awaitable<void> {
    $h = new IO\MemoryHandle('herpderp');
    expect(await $h->readAllAsync())->toEqual('herpderp');
    $h->reset('foobar');
    expect(await $h->readAllAsync())->toEqual('foobar');
    $h->seek(0);
    expect(await $h->readAllAsync())->toEqual('foobar');
  }

  public async function testClose(): Awaitable<void> {
    $h = new IO\MemoryHandle('herp', IO\MemoryHandleWriteMode::APPEND);
    $h->close();
    expect($h->getBuffer())->toEqual('herp');
    $ex = expect(async () ==> await $h->readFixedSizeAsync(1024))->toThrow(
      OS\ErrnoException::class,
    );
    expect($ex->getErrno())->toEqual(OS\Errno::EBADF);
    $h->reset('herp');
    await $h->writeAllowPartialSuccessAsync('derp');
    $h->seek(0);
    expect(await $h->readAllAsync(1024))->toEqual('herpderp');
  }
}
