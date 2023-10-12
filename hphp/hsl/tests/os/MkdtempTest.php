<?hh // strict
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

final class MkdtempTest extends HackTest {
  public function testBasicUsage(): void {
    $pattern = Str\strip_suffix(sys_get_temp_dir(), '/').'/hsl-test-XXXXXX';
    $tempdir = OS\mkdtemp($pattern);
    expect($tempdir)->toNotEqual(
      $pattern,
      'expected literal `X` to be replaced',
    );
    $prefix = Str\strip_suffix($pattern, 'XXXXXX');
    expect(Str\starts_with($tempdir, $prefix))->toBeTrue(
      'tempdir and pattern do not share a prefix',
    );
    expect(is_dir($tempdir))->toBeTrue();
    expect((stat($tempdir)['mode']) & 0777)->toEqual(0700);
    rmdir($tempdir);
  }

  public function testTooFewPlaceholders(): void {
    $pattern = Str\strip_suffix(sys_get_temp_dir(), '/').'/hsl-test-XXX';
    $ex = expect(() ==> OS\mkdtemp($pattern))->toThrow(
      OS\ErrnoException::class,
    );
    expect($ex->getErrno())->toEqual(OS\Errno::EINVAL);
  }

  public function testNoParentDirectory(): void {
    $pattern = '/idonotexist/foo.XXXXXX';
    expect(() ==> OS\mkdtemp($pattern))->toThrow(OS\NotFoundException::class);
  }
}
