<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the hphp/hsl/ subdirectory of this source tree.
 *
 */

use namespace HH\Lib\{File, OS, PseudoRandom, Str};

use function HH\__Private\MiniTest\expect;
use type HH\__Private\MiniTest\HackTest;

final class FileTest extends HackTest {
  public async function testExclusiveOpen(): Awaitable<void> {
    $filename = sys_get_temp_dir().'/'.bin2hex(random_bytes(16));
    $f1 = File\open_write_only($filename, File\WriteMode::MUST_CREATE);
    await $f1->writeAllowPartialSuccessAsync('Hello, world!');
    $e = expect(
      () ==> File\open_write_only($filename, File\WriteMode::MUST_CREATE),
    )->toThrow(OS\ErrnoException::class);
    expect($e->getErrno())->toEqual(OS\Errno::EEXIST);
    $f1->close();

    $f2 = File\open_read_only($filename);
    $content = await $f2->readAllowPartialSuccessAsync();
    $f2->close();
    expect($content)->toEqual('Hello, world!');

    unlink($filename);
  }

  public async function testTemporaryFile(): Awaitable<void> {
    using ($tf = File\temporary_file()) {
      $f = $tf->getHandle();
      $path = $f->getPath();
      // Make sure we didn't get the template
      expect(file_exists($path))->toBeTrue();
      expect($path)->toNotContainSubstring('XXXXXX');

      // Make sure it works :)
      await $f->writeAllowPartialSuccessAsync('Hello, world');
      $content = file_get_contents($path);
      expect($content)->toEqual('Hello, world');

      expect(Str\starts_with($path, sys_get_temp_dir()))->toBeTrue();
      $mode = stat($path)['mode'];
      expect($mode & 0777)->toEqual(
        0600,
        'File should only be readable/writable by current user',
      );
    }
    expect(file_exists($path))->toBeFalse();

    using ($tf = File\temporary_file('foo', '.bar')) {
      $path = $tf->getHandle()->getPath();
      expect($path)->toContainSubstring('/foo');
      expect(Str\ends_with($path, '.bar'))->toBeTrue();
      expect(Str\ends_with($path, '..bar'))->toBeFalse();
    }

    $dir = sys_get_temp_dir().'/hsl-test-'.PseudoRandom\int(0, 99999999);
    mkdir($dir);
    using ($tf = File\temporary_file($dir.'/foo')) {
      expect(Str\starts_with($tf->getHandle()->getPath(), $dir.'/foo'))
        ->toBeTrue();
    }
  }

  public async function testLeakyTemporaryFile(): Awaitable<void> {
    $tf1 = File\leaky_temporary_file();
    expect($tf1->getPath())->toNotContainSubstring('XXXXXX');

    $tf2 = File\leaky_temporary_file();
    expect($tf1->getPath())->toNotEqual($tf2->getPath());
    expect(\file_exists($tf1->getPath()))->toBeTrue();
    $tf1->close();
    expect(\file_exists($tf1->getPath()))->toBeTrue();
    await $tf2->writeAllAsync('hello');
    $tf2->close();

    // Make sure that the write completed, and no deletions or truncates
    // happened
    expect(\file_get_contents($tf2->getPath()))->toEqual('hello');

    \unlink($tf1->getPath());
    \unlink($tf2->getPath());
  }

  public async function testMultipleReads(): Awaitable<void> {
    using ($tf = File\temporary_file()) {
      $f = $tf->getHandle();
      // 10MB is hopefully small enough to not make test infra sad, but
      // way bigger than any reasonable IO buffer size
      $a = Str\repeat('a', 10 * 1024 * 1024);
      $b = Str\repeat('b', 10 * 1024 * 1024);
      $c = Str\repeat('c', 10 * 1024 * 1024);
      await $f->writeAllowPartialSuccessAsync($a.$b.$c);

      $fr = File\open_read_only($f->getPath());
      // FIXME: autoclose
      concurrent {
        $r1 = await $fr->readAllowPartialSuccessAsync(10 * 1024 * 1024);
        $r2 = await $fr->readAllowPartialSuccessAsync(10 * 1024 * 1024);
        $r3 = await $fr->readAllowPartialSuccessAsync(10 * 1024 * 1024);
      }
      // Strong guarantees:
      expect($r1 === $a || $r2 === $a || $r3 === $a)->toBeTrue();
      expect($r1 === $b || $r2 === $b || $r3 === $b)->toBeTrue();
      expect($r1 === $c || $r2 === $c || $r3 === $c)->toBeTrue();
      expect($r1)->toNotEqual($r2);
      expect($r1)->toNotEqual($r3);
      expect($r2)->toNotEqual($r3);
      // NOT GUARANTEED BY HSL API; dependent on eager execution and undefined
      // or semi-defined ordering behavior. Testing here though as we at least
      // want to be aware if an HHVM change changes the behavior here.
      expect($r1)->toEqual($a);
      expect($r2)->toEqual($b);
      expect($r3)->toEqual($c);
    }
  }

  public async function testOpenWithTruncate(): Awaitable<void> {
    using $tf = File\temporary_file();
    $f = $tf->getHandle();
    await $f->writeAllowPartialSuccessAsync('Hello, world');

    $path = $f->getPath();
    $fr = File\open_read_only($path);
    $content = await $fr->readAllowPartialSuccessAsync();
    $fr->close();

    expect($content)->toEqual('Hello, world');

    expect(file_get_contents($path))->toEqual('Hello, world');

    $f = File\open_write_only($path, File\WriteMode::TRUNCATE);
    await $f->writeAllowPartialSuccessAsync('Foo bar');
    expect(file_get_contents($path))->toEqual('Foo bar');
    $f->close();
  }

  public async function testAppend(): Awaitable<void> {
    using $tf = File\temporary_file();
    $f = $tf->getHandle();
    await $f->writeAllowPartialSuccessAsync('Hello, world');

    $path = $f->getPath();
    $f = File\open_write_only($path, File\WriteMode::APPEND);
    await $f->writeAllowPartialSuccessAsync("\nGoodbye, cruel world");
    $f->close();

    expect(file_get_contents($path))->toEqual(
      "Hello, world\nGoodbye, cruel world",
    );
  }

  public async function testLock(): Awaitable<void> {
    using $tf = File\temporary_file();
    $f = $tf->getHandle();
    $path = $f->getPath();

    // With a shared lock held open...
    using ($f->tryLockx(File\LockType::SHARED)) {
      $f2 = File\open_read_only($path);
      using ($f2->tryLockx(File\LockType::SHARED)) {
      }
      $f2->close();
      // Non-disposable as we need to put it in a lambda
      $f3 = File\open_read_only($path);
      expect(() ==> {
        using ($f3->tryLockx(File\LockType::EXCLUSIVE)) {
        }
      })->toThrow(File\AlreadyLockedException::class);
      $f3->close();
    }

    // With an exclusive lock held open...
    using ($f->tryLockx(File\LockType::EXCLUSIVE)) {
      $f4 = File\open_read_only($path);
      expect(() ==> {
        using ($f4->tryLockx(File\LockType::SHARED)) {
        }
      })->toThrow(File\AlreadyLockedException::class);
      $f4->close();
    }
  }

  public function testEarlyClosedDisposables(): void {
    using $tf = File\temporary_file();
    using ($tf->getHandle()->tryLockx(File\LockType::SHARED)) {
      $tf->getHandle()->close();
    }
    // Expectations:
    // - the lock's __dispose didn't throw
    // - the temporary file's __dispose didn't throw
  }

  public async function testTruncate(): Awaitable<void> {
    $filename = sys_get_temp_dir().'/'.bin2hex(random_bytes(16));
    $f = File\open_write_only($filename);
    using ($f->closeWhenDisposed()) {
      await $f->writeAllAsync('Hello, World!');
    }
    $f = File\open_read_write($filename);
    using ($f->closeWhenDisposed()) {
      $f->truncate(5);
      expect(await $f->readAllAsync())->toEqual('Hello');
      $f->truncate(2);
      expect($f->readImpl())->toEqual('');
      $f->seek(0);
      expect(await $f->readAllAsync())->toEqual('He');
    }
  }
}
