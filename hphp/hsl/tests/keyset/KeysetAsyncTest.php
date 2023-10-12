<?hh // strict
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the hphp/hsl/ subdirectory of this source tree.
 *
 */

use namespace HH\Lib\{Keyset, Str, Vec};
use function HH\__Private\MiniTest\expect;
use type HH\__Private\MiniTest\{DataProvider, HackTest};

final class KeysetAsyncTest extends HackTest {

  public static function provideTestFromAsync(): varray<mixed> {
    return varray[
      tuple(
        Vector {
          async {return 'the';},
          async {return 'quick';},
          async {return 'brown';},
          async {return 'fox';},
        },
        keyset['the', 'quick', 'brown', 'fox'],
      ),
      tuple(
        Map {
          'foo' => async {return 1;},
          'bar' => async {return 2;},
        },
        keyset[1, 2],
      ),
      tuple(
        HackLibTestTraversables::getIterator(varray[
          async {return 'the';},
          async {return 'quick';},
          async {return 'brown';},
          async {return 'fox';},
        ]),
        keyset['the', 'quick', 'brown', 'fox'],
      ),
    ];
  }

  <<DataProvider('provideTestFromAsync')>>
  public async function testFromAsync<Tv as arraykey>(
    Traversable<Awaitable<Tv>> $awaitables,
    keyset<Tv> $expected,
  ): Awaitable<void> {
    $actual = await Keyset\from_async($awaitables);
    expect($actual)->toEqual($expected);
  }

  public static function provideTestMapAsync(): varray<mixed> {
    return varray[
      tuple(
        keyset[1,2,3],
        async ($num) ==> $num * 2,
        keyset[2,4,6],
      ),
      tuple(
        vec[1,1,1,2,2,3],
        async ($num) ==> $num * 2,
        keyset[2,4,6],
      ),
      tuple(
        varray['dan', 'danny', 'daniel'],
        async ($word) ==> Str\reverse($word),
        keyset['nad', 'ynnad', 'leinad'],
      ),
    ];
  }

  <<DataProvider('provideTestMapAsync')>>
  public async function testMapAsync<Tv>(
    Traversable<Tv> $traversable,
    (function(Tv): Awaitable<arraykey>) $async_func,
    keyset<arraykey> $expected,
  ): Awaitable<void> {
    $actual = await Keyset\map_async($traversable, $async_func);
    expect($actual)->toEqual($expected);
  }

  public static function provideTestFilterAsync(
  ): vec<(
    Container<arraykey>,
    (function(arraykey): Awaitable<bool>),
    keyset<arraykey>,
  )> {
    return vec[
      tuple(
        darray[
          2 => 'two',
          4 => 'four',
          6 => 'six',
          8 => 'eight',
        ],
        async ($word) ==> Str\length((string)$word) % 2 === 1,
        keyset['two', 'six', 'eight'],
      ),
      tuple(
        Vector {'jumped', 'over', 'jumped'},
        async ($word) ==> Str\length((string)$word) % 2 === 0,
        keyset['jumped', 'over'],
      ),
      tuple(
        Set {'the', 'quick', 'brown', 'fox', 'jumped', 'over'},
        async ($word) ==> Str\length((string)$word) % 2 === 0,
        keyset['jumped', 'over'],
      ),
    ];
  }

  <<DataProvider('provideTestFilterAsync')>>
  public async function testFilterAsync<Tv as arraykey>(
    Container<Tv> $traversable,
    (function(Tv): Awaitable<bool>) $async_predicate,
    keyset<Tv> $expected,
  ): Awaitable<void> {
    $actual = await Keyset\filter_async($traversable, $async_predicate);
    expect($actual)->toEqual($expected);
  }
   
  public static function provideTestPartitionAsync(): varray<mixed> {
    return varray[
      tuple(
        Vec\concat(Vec\range(1, 10), Vec\range(1, 10)),
        async $n ==> $n % 2 === 0,
        tuple(keyset[2, 4, 6, 8, 10], keyset[1, 3, 5, 7, 9]),
      ),
      tuple(
        dict['one' => 1, 'two' => 2, 'three' => 3, 'four' => 4, 'five' => 5],
        async $n ==> $n % 2 === 0,
        tuple(keyset[2, 4], keyset[1, 3, 5]),
      ),
    ];
  }
 
  <<DataProvider('provideTestPartitionAsync')>>
  public async function testPartitionAsync<Tv as arraykey>(
    Container<Tv> $container,
    (function(Tv): Awaitable<bool>) $async_predicate,
    (vec<Tv>, vec<Tv>) $expected,
  ): Awaitable<void> {
    $actual = await Keyset\partition_async($container, $async_predicate);
    expect($actual)->toEqual($expected);
  }
}
