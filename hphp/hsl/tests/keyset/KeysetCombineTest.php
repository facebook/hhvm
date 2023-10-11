<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the hphp/hsl/ subdirectory of this source tree.
 *
 */

use namespace HH\Lib\Keyset;
use function HH\__Private\MiniTest\expect;
use type HH\__Private\MiniTest\{DataProvider, HackTest};

final class KeysetCombineTest extends HackTest {

  public static function provideTestUnion(): varray<mixed> {
    return varray[
      tuple(
        varray[],
        varray[
          vec[],
        ],
        keyset[],
      ),
      tuple(
        keyset[1, 2, 3],
        varray[
          Vector {'the', 'quick', 'brown'},
          HackLibTestTraversables::getKeyedIterator(darray[
            'the' => 'the',
            'quick' => 'quick',
            'brown' => 'brown',
            'fox' => 'jumped',
          ]),
        ],
        keyset[1, 2, 3, 'the', 'quick', 'brown', 'jumped'],
      ),
    ];
  }

  <<DataProvider('provideTestUnion')>>
  public function testUnion<Tv as arraykey>(
    Traversable<Tv> $first,
    Container<Container<Tv>> $rest,
    keyset<Tv> $expected,
  ): void {
    expect(Keyset\union($first, ...$rest))->toEqual($expected);
  }
}
