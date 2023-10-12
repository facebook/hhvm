<?hh // strict
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the hphp/hsl/ subdirectory of this source tree.
 *
 */

use namespace HH\Lib\Vec;
use function HH\__Private\MiniTest\expect;
use type HH\__Private\MiniTest\{DataProvider, HackTest};

final class VecDivideTest extends HackTest {

  public static function providePartition(): varray<mixed> {
    return varray[
      tuple(
        Vec\range(1, 10),
        $n ==> $n % 2 === 0,
        tuple(
          vec[2, 4, 6, 8, 10],
          vec[1, 3, 5, 7, 9],
        ),
      ),
      tuple(
        HackLibTestTraversables::getIterator(Vec\range(1, 10)),
        $n ==> $n % 2 === 0,
        tuple(
          vec[2, 4, 6, 8, 10],
          vec[1, 3, 5, 7, 9],
        ),
      ),
    ];
  }

  <<DataProvider('providePartition')>>
  public function testPartition<Tv>(
    Traversable<Tv> $traversable,
    (function(Tv): bool) $predicate,
    (vec<Tv>, vec<Tv>) $expected,
  ): void {
    expect(Vec\partition($traversable, $predicate))->toEqual($expected);
  }
}
