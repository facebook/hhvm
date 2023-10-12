<?hh // strict
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

final class KeysetTransformTest extends HackTest {

  public static function provideTestChunk(): varray<mixed> {
    return varray[
      tuple(
        Map {},
        10,
        vec[],
      ),
      tuple(
        varray[0, 1, 2, 3, 4],
        2,
        vec[
          keyset[0, 1],
          keyset[2, 3],
          keyset[4],
        ],
      ),
      tuple(
        HackLibTestTraversables::getKeyedIterator(
          darray['foo' => 'bar', 'baz' => 'qux'],
        ),
        1,
        vec[
          keyset['bar'],
          keyset['qux'],
        ],
      ),
      tuple(
        vec[0, 0, 1, 1, 1, 2, 3, 4, 5, 6],
        3,
        vec[
          keyset[0, 1],
          keyset[1, 2],
          keyset[3, 4, 5],
          keyset[6],
        ],
      ),
    ];
  }

  <<DataProvider('provideTestChunk')>>
  public function testChunk<Tv as arraykey>(
    Traversable<Tv> $traversable,
    int $size,
    vec<keyset<Tv>> $expected,
  ): void {
    expect(Keyset\chunk($traversable, $size))->toEqual($expected);
  }

  public static function provideTestMap(): varray<mixed> {
    $doubler = $x ==> $x * 2;
    return varray[
      tuple(
        varray[],
        $doubler,
        keyset[],
      ),
      tuple(
        varray[1],
        $doubler,
        keyset[2],
      ),
      tuple(
        range(10, 15),
        $doubler,
        keyset[20, 22, 24, 26, 28, 30],
      ),
      tuple(
        varray['a'],
        $x ==> $x. ' buzz',
        keyset['a buzz'],
      ),
      tuple(
        varray['a', 'bee', 'a bee'],
        $x ==> $x. ' buzz',
        keyset['a buzz', 'bee buzz', 'a bee buzz'],
      ),
      tuple(
        dict[
          'donald' => 'duck',
          'daffy' => 'duck',
          'mickey' => 'mouse',
        ],
        $s ==> Str\reverse($s),
        keyset['kcud', 'kcud', 'esuom'],
      ),
      tuple(
        Map {'donald' => 'duck', 'daffy' => 'duck', 'mickey' => 'mouse'},
        $s ==> Str\reverse($s),
        keyset['kcud', 'kcud', 'esuom'],
      ),
      tuple(
        Vector {10, 20},
        $doubler,
        keyset[20, 40],
      ),
      tuple(
        Set {10, 20},
        $doubler,
        keyset[20, 40],
      ),
      tuple(
        keyset[10, 20],
        $doubler,
        keyset[20, 40],
      ),
      tuple(
        HackLibTestTraversables::getIterator(varray[1, 2, 3]),
        $doubler,
        keyset[2, 4, 6],
      ),
      tuple(
        HackLibTestTraversables::getKeyedIterator(darray[10 => 1, 20 => 2, 30 => 3]),
        $doubler,
        keyset[2, 4, 6],
      ),
    ];
  }

  <<DataProvider('provideTestMap')>>
  public function testMap<Tv1, Tv2 as arraykey>(
    Traversable<Tv1> $traversable,
    (function(Tv1): Tv2) $value_func,
    keyset<Tv2> $expected,
  ): void {
    expect(Keyset\map($traversable, $value_func))->toEqual($expected);
  }

  public static function provideTestMapWithKey(): varray<mixed> {
    return varray[
      tuple(
        varray[],
        ($a, $b) ==> null,
        keyset[],
      ),
      tuple(
        Vector {'the', 'quick', 'brown', 'fox'},
        ($k, $v) ==> (string)$k.$v,
        keyset['0the', '1quick', '2brown', '3fox'],
      ),
      tuple(
        HackLibTestTraversables::getKeyedIterator(range(1, 5)),
        ($k, $v) ==> $v * $k,
        keyset[0, 2, 6, 12, 20],
      ),
      tuple(
        range(1, 6),
        ($k, $v) ==> ($k + $v) % 5,
        keyset[1, 3, 0, 2, 4],
      ),
    ];
  }

  <<DataProvider('provideTestMapWithKey')>>
  public function testMapWithKey<Tk, Tv1, Tv2 as arraykey>(
    KeyedTraversable<Tk, Tv1> $traversable,
    (function(Tk, Tv1): Tv2) $value_func,
    keyset<Tv2> $expected,
  ): void {
    expect(Keyset\map_with_key($traversable, $value_func))
      ->toEqual($expected);
  }

  public static function provideTestFlatten(
  ): vec<(Traversable<Traversable<arraykey>>, keyset<arraykey>)> {
    return vec[
      tuple(
        vec[keyset[1,2], keyset[2,3,4]],
        keyset[1,2,3,4],
      ),
      tuple(
        vec[keyset[1]],
        keyset[1],
      ),
      tuple(
        vec[],
        keyset[],
      ),
      tuple(
        vec[keyset[], keyset[]],
        keyset[],
      ),
      tuple(
        vec[vec[1,2],vec[2,3]],
        keyset[1,2,3],
      ),
      tuple(
        dict['a' => keyset['apple', 'banana'], 'b' => vec['grape']],
        keyset['apple', 'banana', 'grape'],
      ),
      tuple(
        varray[
          varray[1, 2, 3],
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

  <<DataProvider('provideTestFlatten')>>
  public function testFlatten<Tv as arraykey>(
    Traversable<Container<Tv>> $traversables,
    keyset<Tv> $expected,
  ): void {
    expect(Keyset\flatten($traversables))->toEqual($expected);
  }
}
