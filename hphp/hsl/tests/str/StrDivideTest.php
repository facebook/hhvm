<?hh // strict
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the hphp/hsl/ subdirectory of this source tree.
 *
 */

use namespace HH\Lib\Str;
use function HH\__Private\MiniTest\expect;
use type HH\__Private\MiniTest\{DataProvider, HackTest};

final class StrDivideTest extends HackTest {

  public static function provideChunk(): varray<mixed> {
    return varray[
      tuple(
        'hello',
        1,
        vec['h', 'e', 'l', 'l', 'o'],
      ),
      tuple(
        'hello',
        10,
        vec['hello'],
      ),
      tuple(
        'hello',
        2,
        vec['he', 'll', 'o'],
      ),
    ];
  }

  <<DataProvider('provideChunk')>>
  public function testChunk(
    string $string,
    int $chunk_size,
    vec<string> $expected,
  ): void {
    expect(Str\chunk($string, $chunk_size))->toEqual($expected);
  }

  public static function provideSplit(): varray<mixed> {
    return varray[
      tuple(
        '',
        '',
        null,
        vec[''],
      ),
      tuple(
        '',
        'hello',
        null,
        vec['h', 'e', 'l', 'l', 'o'],
      ),
      tuple(
        '',
        'hello',
        300,
        vec['h', 'e', 'l', 'l', 'o'],
      ),
      tuple(
        '',
        'hello',
        3,
        vec['h', 'e', 'llo'],
      ),
      tuple(
        '',
        'hello',
        1,
        vec['hello'],
      ),
      tuple(
        '-',
        'hello',
        null,
        vec['hello'],
      ),
      tuple(
        '-',
        '-hello',
        null,
        vec['', 'hello'],
      ),
      tuple(
        '-',
        'hello-',
        null,
        vec['hello', ''],
      ),
      tuple(
        ' ',
        'the quick brown fox jumped',
        null,
        vec['the', 'quick', 'brown', 'fox', 'jumped'],
      ),
      tuple(
        ' ',
        'the quick brown fox jumped',
        3,
        vec['the', 'quick', 'brown fox jumped'],
      ),
    ];
  }

  <<DataProvider('provideSplit')>>
  public function testSplit(
    string $delimiter,
    string $string,
    ?int $limit,
    vec<string> $expected,
  ): void {
    expect(Str\split($string, $delimiter, $limit))
      ->toEqual($expected);
  }

}
