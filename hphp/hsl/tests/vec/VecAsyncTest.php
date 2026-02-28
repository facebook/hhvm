<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the hphp/hsl/ subdirectory of this source tree.
 *
 */

use namespace HH\Lib\{Str, Vec};
use function HH\__Private\MiniTest\expect;
use type HH\__Private\MiniTest\{DataProvider, HackTest};

final class VecAsyncTest extends HackTest {

  public static function provideTestFromAsync(): varray<mixed> {
    return vec[
      tuple(
        Vector {
          async {return 'the';},
          async {return 'quick';},
          async {return 'brown';},
          async {return 'fox';},
        },
        vec['the', 'quick', 'brown', 'fox'],
      ),
      tuple(
        Map {
          'foo' => async {return 1;},
          'bar' => async {return 2;},
        },
        vec[1, 2],
      ),
      tuple(
        HackLibTestTraversables::getIterator(vec[
          async {return 'the';},
          async {return 'quick';},
          async {return 'brown';},
          async {return 'fox';},
        ]),
        vec['the', 'quick', 'brown', 'fox'],
      ),
    ];
  }

  <<DataProvider('provideTestFromAsync')>>
  public async function testFromAsync<Tv>(
    Traversable<Awaitable<Tv>> $awaitables,
    vec<Tv> $expected,
  ): Awaitable<void> {
    $actual = await Vec\from_async($awaitables);
    expect($actual)->toEqual($expected);
  }

  public static function provideTestFilterAsync(): varray<mixed> {
    return vec[
      tuple(
        dict[
          2 => 'two',
          4 => 'four',
          6 => 'six',
          8 => 'eight',
        ],
        async ($word) ==> Str\length($word) % 2 === 1,
        vec['two', 'six', 'eight'],
      ),
      tuple(
        Vector {'the', 'quick', 'brown', 'fox', 'jumped', 'over'},
        async ($word) ==> Str\length($word) % 2 === 0,
        vec['jumped', 'over'],
      ),
    ];
  }

  <<DataProvider('provideTestFilterAsync')>>
  public async function testFilterAsync<Tv>(
    Container<Tv> $container,
    (function(Tv): Awaitable<bool>) $value_predicate,
    vec<Tv> $expected,
  ): Awaitable<void> {
    $actual = await Vec\filter_async($container, $value_predicate);
    expect($actual)->toEqual($expected);
  }

  public static function provideTestMapAsync(): varray<mixed> {
    return vec[
      tuple(
        Vector {'the', 'quick', 'brown', 'fox'},
        async ($word) ==> Str\reverse($word),
        vec['eht', 'kciuq', 'nworb', 'xof'],
      ),
      tuple(
        HackLibTestTraversables::getIterator(
          vec['the', 'quick', 'brown', 'fox'],
        ),
        async ($word) ==> Str\reverse($word),
        vec['eht', 'kciuq', 'nworb', 'xof'],
      ),
      tuple(
        dict['one' => 'uno', 'two' => 'due', 'three' => 'tre'],
        async ($word) ==> Str\reverse($word),
        vec['onu', 'eud', 'ert'],
      ),
    ];
  }

  <<DataProvider('provideTestMapAsync')>>
  public async function testMapAsync<Tk, Tv>(
    Traversable<Tk> $keys,
    (function(Tk): Awaitable<Tv>) $async_func,
    vec<Tv> $expected,
  ): Awaitable<void> {
    $actual = await Vec\map_async($keys, $async_func);
    expect($actual)->toEqual($expected);
  }

  public static function provideTestMapWithKeyAsync(): vec<mixed> {
    return vec[
      tuple(vec[], async ($a, $b) ==> null, vec[]),
      tuple(
        vec[1, 2, 3],
        async ($k, $v) ==> (string)$k.$v,
        vec['01', '12', '23'],
      ),
      tuple(
        Vector {'the', 'quick', 'brown', 'fox'},
        async ($k, $v) ==> (string)$k.$v,
        vec[
          '0the',
          '1quick',
          '2brown',
          '3fox',
        ],
      ),
      tuple(
        HackLibTestTraversables::getKeyedIterator(Vec\range(1, 5)),
        async ($k, $v) ==> $k * $v,
        vec[
          0,
          2,
          6,
          12,
          20,
        ],
      ),
    ];
  }

  <<DataProvider('provideTestMapWithKeyAsync')>>
  public async function testMapWithKeyAsync<Tk as arraykey, Tv1, Tv2>(
    KeyedTraversable<Tk, Tv1> $traversable,
    (function(Tk, Tv1): Awaitable<Tv2>) $value_func,
    vec<Tv2> $expected,
  ): Awaitable<void> {
    $result = await Vec\map_with_key_async($traversable, $value_func);
    expect($result)->toEqual($expected);
  }

  public static function provideTestPartitionAsync(): varray<mixed> {
    return vec[
      tuple(
        Vec\range(1, 10),
        async $n ==> $n % 2 === 0,
        tuple(vec[2, 4, 6, 8, 10], vec[1, 3, 5, 7, 9]),
      ),
      tuple(
        dict['one' => 1, 'two' => 2, 'three' => 3, 'four' => 4, 'five' => 5],
        async $n ==> $n % 2 === 0,
        tuple(vec[2, 4], vec[1, 3, 5]),
      ),
    ];
  }

  <<DataProvider('provideTestPartitionAsync')>>
  public async function testPartitionAsync<Tv>(
    Container<Tv> $container,
    (function(Tv): Awaitable<bool>) $value_predicate,
    (vec<Tv>, vec<Tv>) $expected,
  ): Awaitable<void> {
    $actual = await Vec\partition_async($container, $value_predicate);
    expect($actual)->toEqual($expected);
  }
}
