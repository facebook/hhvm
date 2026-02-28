<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the hphp/hsl/ subdirectory of this source tree.
 *
 */

use namespace HH\Lib\{Keyset, Str};
use function HH\__Private\MiniTest\expect;
use type HH\__Private\MiniTest\{DataProvider, HackTest};

final class KeysetOrderTest extends HackTest {

  public static function provideSort(): varray<mixed> {
    return vec[
      tuple(
        vec['the', 'quick', 'brown', 'fox'],
        null,
        keyset['brown', 'fox', 'quick', 'the'],
      ),
      tuple(
        Vector {'the', 'quick', 'brown', 'fox'},
        /* HH_FIXME[4297] The type of the lambda argument(s) could not be inferred */
        ($a, $b) ==> Str\compare($a[1],$b[1]),
        keyset['the', 'fox', 'brown', 'quick'],
      ),
      tuple(
        vec[8, 6, 7, 5, 3, 0, 9],
        null,
        keyset[0, 3, 5, 6, 7, 8, 9],
      ),
      tuple(
        HackLibTestTraversables::getIterator(vec[8, 6, 7, 5, 3, 0, 9]),
        null,
        keyset[0, 3, 5, 6, 7, 8, 9],
      ),
    ];
  }

  <<DataProvider('provideSort')>>
  public function testSort<Tv as arraykey>(
    Traversable<Tv> $traversable,
    ?(function(Tv, Tv): int) $comparator,
    keyset<Tv> $expected,
  ): void {
    expect(Keyset\sort($traversable, $comparator))->toEqual($expected);
  }

}
