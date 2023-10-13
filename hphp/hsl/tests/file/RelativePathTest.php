<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the hphp/hsl/ subdirectory of this source tree.
 *
 */


use function HH\__Private\MiniTest\expect;
use type HH\__Private\MiniTest\HackTest;
use namespace HH\Lib\_Private\_File;
final class RelativePathTest extends HackTest {

  public async function testBasic(): Awaitable<void> {
    $real_path = _File\relative_path(
      'baz',
      'foo/bar',
    );
    expect($real_path)->toEqual('foo/bar/baz');
  }

  public async function testDot(): Awaitable<void> {
    $real_path = \realpath(_File\relative_path(
      './DictSelectTest.php',
      _File\relative_path('../dict', __DIR__),
    ));
    expect($real_path)->toContainSubstring('/tests/dict/DictSelectTest.php');
  }

  public async function testSlash(): Awaitable<void> {
    $path = _File\relative_path('/bin/sh', 'path/to/whatever');
    expect($path)->toEqual('/bin/sh');
  }

  public async function testNull(): Awaitable<void> {
    $path = _File\relative_path('path/to/file', null);
    expect($path)->toEqual('path/to/file');
  }

}
