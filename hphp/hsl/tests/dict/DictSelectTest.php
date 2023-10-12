<?hh // strict
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the hphp/hsl/ subdirectory of this source tree.
 *
 */

use namespace HH\Lib\{Dict, Str, Vec};
use function HH\__Private\MiniTest\expect;
use type HH\__Private\MiniTest\{DataProvider, HackTest};

final class DictSelectTest extends HackTest {

  public static function provideTestDiffByKey(): varray<mixed> {
    return varray[
      tuple(
        darray[],
        Vec\range(0, 100),
        darray[],
        dict[],
      ),
      tuple(
        darray[1 => 1, 2 => 2, 3 => 3],
        darray[],
        darray[],
        dict[1 => 1, 2 => 2, 3 => 3],
      ),
      tuple(
        dict['foo' => 'bar', 'baz' => 'qux'],
        Map {'foo' => 4},
        darray[],
        dict['baz' => 'qux'],
      ),
      tuple(
        Vec\range(0, 9),
        dict[2 => 4, 4 => 8, 8 => 16],
        varray[
          Map {1 => 1, 2 => 2},
          HackLibTestTraversables::getKeyedIterator(Vec\range(0, 3)),
        ],
        dict[5 => 5, 6 => 6, 7 => 7, 9 => 9],
      ),
    ];
  }

  <<DataProvider('provideTestDiffByKey')>>
  public function testDiffByKey<Tk1 as arraykey, Tk2 as arraykey, Tv>(
    KeyedTraversable<Tk1, Tv> $first,
    KeyedTraversable<Tk2, mixed> $second,
    Container<KeyedContainer<Tk2, mixed>> $rest,
    dict<Tk1, Tv> $expected,
  ): void {
    expect(Dict\diff_by_key($first, $second, ...$rest))->toEqual($expected);
  }

  public static function provideDrop(): varray<mixed> {
    return varray[
      tuple(
        dict[],
        5,
        dict[],
      ),
      tuple(
        Vector {0, 1, 2, 3},
        0,
        dict[
          0 => 0,
          1 => 1,
          2 => 2,
          3 => 3,
        ],
      ),
      tuple(
        HackLibTestTraversables::getKeyedIterator(darray[
          'foo' => 'oof',
          'bar' => 'rab',
          'baz' => 'zab',
          'qux' => 'xuq',
        ]),
        3,
        dict[
          'qux' => 'xuq',
        ],
      ),
      tuple(
        Map {
          'foo' => 'oof',
          'bar' => 'rab',
          'baz' => 'zab',
          'qux' => 'xuq',
          'yap' => 'pay',
        },
        10,
        dict[],
      ),
    ];
  }

  <<DataProvider('provideDrop')>>
  public function testDrop<Tk as arraykey, Tv>(
    KeyedTraversable<Tk, Tv> $traversable,
    int $n,
    dict<Tk, Tv> $expected,
  ): void {
    expect(Dict\drop($traversable, $n))->toEqual($expected);
  }

  public static function provideTestFilter(): varray<mixed> {
    return varray[
      tuple(
        dict[],
        $x ==> true,
        dict[],
      ),
      tuple(
        dict[],
        $x ==> false,
        dict[],
      ),
      tuple(
        dict[0 => 1],
        $x ==> true,
        dict[0 => 1],
      ),
      tuple(
        dict[0 => 1],
        $x ==> false,
        dict[],
      ),
      tuple(
        dict(Vec\range(1, 10)),
        $x ==> $x % 2 === 0,
        dict[1 => 2, 3 => 4, 5 => 6, 7 => 8, 9 => 10],
      ),
      tuple(
        Map {'donald' => 'duck', 'daffy' => 'duck', 'mickey' => 'mouse'},
        $x ==> $x === 'duck',
        dict['donald' => 'duck', 'daffy' => 'duck'],
      ),
      tuple(
        HackLibTestTraversables::getIterator(Vec\range(1, 5)),
        $x ==> $x % 2 === 0,
        dict[1 => 2, 3 => 4],
      ),
    ];
  }

  <<DataProvider('provideTestFilter')>>
  public function testFilter<Tk as arraykey, Tv>(
    KeyedTraversable<Tk, Tv> $traversable,
    (function(Tv): bool) $value_predicate,
    dict<Tk, Tv> $expected,
  ): void {
    expect(Dict\filter($traversable, $value_predicate))->toEqual($expected);
  }

  public static function provideTestFilterWithKey(): varray<mixed> {
    return varray[
      tuple(
        dict[],
        ($k, $v) ==> true,
        dict[],
      ),
      tuple(
        dict[],
        ($k, $v) ==> false,
        dict[],
      ),
      tuple(
        dict[0 => 1],
        ($k, $v) ==> true,
        dict[0 => 1],
      ),
      tuple(
        dict[0 => 1],
        ($k, $v) ==> false,
        dict[],
      ),
      tuple(
        dict(Vec\range(1, 10)),
        ($k, $v) ==> $k % 2 === 0 && $v % 2 === 0,
        dict[],
      ),
      tuple(
        dict(Vec\range(1, 10)),
        ($k, $v) ==> $k === $v - 1,
        dict(Vec\range(1, 10)),
      ),
      tuple(
        Map {'donald' => 'duck', 'daffy' => 'duck', 'mickey' => 'mouse'},
        ($k, $v) ==> $v === 'duck' && $k === 'donald',
        dict['donald' => 'duck'],
      ),
      tuple(
        HackLibTestTraversables::getIterator(Vec\range(1, 5)),
        ($k, $v) ==> $v % 2 === 0,
        dict[1 => 2, 3 => 4],
      ),
    ];
  }

  <<DataProvider('provideTestFilterWithKey')>>
  public function testFilterWithKey<Tk as arraykey, Tv>(
    KeyedTraversable<Tk, Tv> $traversable,
    (function(Tk, Tv): bool) $predicate,
    dict<Tk, Tv> $expected,
  ): void {
    expect(Dict\filter_with_key($traversable, $predicate))->toEqual($expected);
  }

  public function testFilterWithoutPredicate(): void {
    expect(Dict\filter(darray[
      0 => 0,
      3 => 3,
      2 => null,
      4 => 5,
      30 => false,
      40 => 40,
      50 => '',
      60 => '0',
      70 => 'win!',
    ]))->toEqual(dict[3 => 3, 4 => 5, 40 => 40, 70 => 'win!']);
  }

  public static function provideTestFilterKeys(): varray<mixed> {
    return varray[
      tuple(
        dict[],
        $x ==> true,
        dict[],
      ),
      tuple(
        dict[],
        $x ==> false,
        dict[],
      ),
      tuple(
        Map {'donald' => 'duck', 'daffy' => 'duck', 'huey' => 'duck'},
        $x ==> Str\starts_with($x, 'd'),
        dict['donald' => 'duck', 'daffy' => 'duck'],
      ),
      tuple(
        dict(Vec\range(1, 10)),
        $x ==> $x % 2 === 0,
        dict[0 => 1, 2 => 3, 4 => 5, 6 => 7, 8 => 9],
      ),
      tuple(
        HackLibTestTraversables::getIterator(Vec\range(1, 5)),
        $x ==> $x % 2 === 0,
        dict[0 => 1, 2 => 3, 4 => 5],
      ),
    ];
  }

  <<DataProvider('provideTestFilterKeys')>>
  public function testFilterKeys<Tk as arraykey, Tv>(
    KeyedTraversable<Tk, Tv> $traversable,
    (function(Tk): bool) $key_predicate,
    dict<Tk, Tv> $expected,
  ): void {
    expect(Dict\filter_keys($traversable, $key_predicate))
      ->toEqual($expected);
  }

  public function testFilterKeysWithoutPredicate(): void {
    expect(Dict\filter_keys(dict[
      '' => 1,
      '0' => 2,
      0 => 3,
      1 => 4,
      'hi' => 5,
    ]))->toEqual(dict[1 => 4, 'hi' => 5]);
  }

  public static function provideTestFilterNulls(): varray<mixed> {
    return varray[
      tuple(
        darray[
          'foo' => null,
          'bar' => null,
          'baz' => null,
        ],
        dict[],
      ),
      tuple(
        Map {
          'foo' => false,
          'bar' => null,
          'baz' => '',
          'qux' => 0,
        },
        dict[
          'foo' => false,
          'baz' => '',
          'qux' => 0,
        ],
      ),
      tuple(
        Vector {
          'foo',
          'bar',
          null,
          'baz',
        },
        dict[
          0 => 'foo',
          1 => 'bar',
          3 => 'baz',
        ],
      ),
      tuple(
        HackLibTestTraversables::getKeyedIterator(darray[
          1 => null,
          2 => varray[],
          3 => '0',
        ]),
        dict[
          2 => varray[],
          3 => '0',
        ],
      ),
    ];
  }

  <<DataProvider('provideTestFilterNulls')>>
  public function testFilterNulls<Tk as arraykey, Tv>(
    KeyedTraversable<Tk, ?Tv> $traversable,
    dict<Tk, Tv> $expected,
  ): void {
    expect(Dict\filter_nulls($traversable))->toEqual($expected);
  }

  public static function provideTestSelectKeys(): varray<mixed> {
    return varray[
      tuple(
        varray[],
        varray[],
        dict[],
      ),
      tuple(
        Map {
          'foo' => 'foo',
          'bar' => 'bar',
          'baz' => 'baz',
        },
        varray['bar'],
        dict[
          'bar' => 'bar',
        ],
      ),
      tuple(
        Vector {'the', 'quick', 'brown', 'fox', 'jumped', 'over', 'the', 'dog'},
        Set {0, 2, 7, 4},
        dict[
          0 => 'the',
          2 => 'brown',
          7 => 'dog',
          4 => 'jumped',
        ],
      ),
    ];
  }

  <<DataProvider('provideTestSelectKeys')>>
  public function testSelectKeys<Tk as arraykey, Tv>(
    KeyedContainer<Tk, Tv> $container,
    Traversable<Tk> $keys,
    dict<Tk, Tv> $expected,
  ): void {
    expect(Dict\select_keys($container, $keys))->toEqual($expected);
  }

  public static function provideTake(): varray<mixed> {
    return varray[
      tuple(
        dict[],
        5,
        dict[],
      ),
      tuple(
        Vector {0, 1, 2, 3, 4},
        0,
        dict[],
      ),
      tuple(
        HackLibTestTraversables::getKeyedIterator(darray[
          'foo' => 'oof',
          'bar' => 'rab',
          'baz' => 'zab',
          'qux' => 'xuq',
        ]),
        3,
        dict[
          'foo' => 'oof',
          'bar' => 'rab',
          'baz' => 'zab',
        ],
      ),
      tuple(
        Map {
          'foo' => 'oof',
          'bar' => 'rab',
          'baz' => 'zab',
          'qux' => 'xuq',
          'yap' => 'pay',
        },
        10,
        dict[
          'foo' => 'oof',
          'bar' => 'rab',
          'baz' => 'zab',
          'qux' => 'xuq',
          'yap' => 'pay',
        ],
      ),
    ];
  }

  <<DataProvider('provideTake')>>
  public function testTake<Tk as arraykey, Tv>(
    KeyedTraversable<Tk, Tv> $traversable,
    int $n,
    dict<Tk, Tv> $expected,
  ): void {
    expect(Dict\take($traversable, $n))->toEqual($expected);
  }

  public function testTakeIter(): void {
    $iter = HackLibTestTraversables::getKeyedIterator(Vec\range(0, 4));
    expect(Dict\take($iter, 2))->toEqual(dict[0=>0, 1=>1]);
    expect(Dict\take($iter, 0))->toEqual(dict[]);
    expect(Dict\take($iter, 2))->toEqual(dict[2=>2, 3=>3]);
    expect(Dict\take($iter, 2))->toEqual(dict[4=>4]);
  }

  public static function provideTestUnique(): varray<mixed> {
    return varray[
      tuple(
        Map {
          'a' => 1,
          'b' => 2,
          'c' => 2,
          'd' => 1,
          'e' => 3,
        },
        dict[
          'd' => 1,
          'c' => 2,
          'e' => 3,
        ],
      ),
    ];
  }

  <<DataProvider('provideTestUnique')>>
  public function testUnique<Tk as arraykey, Tv as arraykey>(
    KeyedTraversable<Tk, Tv> $traversable,
    dict<Tk, Tv> $expected,
  ): void {
    expect(Dict\unique($traversable))->toEqual($expected);
  }

  public static function provideTestUniqueBy(): varray<mixed> {
    $s1 = Set {'foo'};
    $s2 = Set {'bar'};
    $s3 = Set {'foo'};
    $s4 = Set {'baz'};
    return varray[
      tuple(
        Vector {$s1, $s2, $s3, $s4},
        /* HH_FIXME[4297] The type of the lambda argument(s) could not be inferred */
        ($s) ==> $s->firstKey(),
        dict[
          2 => $s3,
          1 => $s2,
          3 => $s4,
        ],
      ),
    ];
  }

  <<DataProvider('provideTestUniqueBy')>>
  public function testUniqueBy<Tk as arraykey, Tv, Ts as arraykey>(
    KeyedContainer<Tk, Tv> $container,
    (function(Tv): Ts) $scalar_func,
    dict<Tk, Tv> $expected,
  ): void {
    expect(Dict\unique_by($container, $scalar_func))->toEqual($expected);
  }
}
