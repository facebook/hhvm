<?hh
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

final class DictTransformTest extends HackTest {

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
          dict[
            0 => 0,
            1 => 1,
          ],
          dict[
            2 => 2,
            3 => 3,
          ],
          dict[
            4 => 4,
          ],
        ],
      ),
      tuple(
        HackLibTestTraversables::getKeyedIterator(
          darray['foo' => 'bar', 'baz' => 'qux'],
        ),
        1,
        vec[
          dict['foo' => 'bar'],
          dict['baz' => 'qux'],
        ],
      ),
    ];
  }

  <<DataProvider('provideTestChunk')>>
  public function testChunk<Tk as arraykey, Tv>(
    KeyedTraversable<Tk, Tv> $traversable,
    int $size,
    vec<dict<Tk, Tv>> $expected,
  ): void {
    expect(Dict\chunk($traversable, $size))->toEqual($expected);
  }

  public static function provideTestCountValues(): varray<mixed> {
    return varray[
      tuple(
        varray[0, '0', 1, 3, 4, 1, 1, 3, '1'],
        dict[
          0 => 1,
          '0' => 1,
          1 => 3,
          3 => 2,
          4 => 1,
          '1' => 1,
        ],
      ),
      tuple(
        Map {
          'donald' => 'duck',
          'bugs' => 'bunny',
          'daffy' => 'duck',
        },
        dict[
          'duck' => 2,
          'bunny' => 1,
        ],
      ),
    ];
  }

  <<DataProvider('provideTestCountValues')>>
  public function testCountValues<Tv as arraykey>(
    Traversable<Tv> $values,
    dict<Tv, int> $expected,
  ): void {
    expect(Dict\count_values($values))->toEqual($expected);
  }

  public static function provideTestFillKeys(): varray<mixed> {
    return varray[
      tuple(
        darray[],
        'foo',
        dict[],
      ),
      tuple(
        Vector {'the', 'quick', 'brown', 'fox'},
        42,
        dict[
          'the' => 42,
          'quick' => 42,
          'brown' => 42,
          'fox' => 42,
        ],
      ),
      tuple(
        HackLibTestTraversables::getIterator(Vec\range(1, 3)),
        'hi',
        dict[
          1 => 'hi',
          2 => 'hi',
          3 => 'hi',
        ],
      ),
    ];
  }

  <<DataProvider('provideTestFillKeys')>>
  public function testFillKeys<Tk as arraykey, Tv>(
    Traversable<Tk> $keys,
    Tv $value,
    dict<Tk, Tv> $expected,
  ): void {
    expect(Dict\fill_keys($keys, $value))->toEqual($expected);
  }

  public static function provideTestFlatten(): dict<
    string,
    (Traversable<KeyedContainer<arraykey, mixed>>, dict<arraykey, mixed>),
  > {
    return dict[
      'all empty' => tuple(dict[], dict[]),
      'flatten multiple' => tuple(
        vec[
          dict[
            'one' => 'one',
            'two' => 'two',
          ],
          dict[
            'three' => 'three',
            'one' => 3,
          ],
          Map {
            'four' => null,
          },
        ],
        dict[
          'one' => 3,
          'two' => 'two',
          'three' => 'three',
          'four' => null,
        ],
      ),
      'flatten multiple (first empty)' => tuple(
        vec[
          dict[],
          dict[
            'three' => 'three',
            'one' => 3,
          ],
          Map {
            'four' => null,
          },
        ],
        dict[
          'three' => 'three',
          'one' => 3,
          'four' => null,
        ],
      ),
      'various KeyedContainer types' => tuple(
        vec[
          dict[
            'foo' => 'foo',
            'bar' => 'bar',
            'baz' => vec[1, 2, 3],
          ],
          dict[
            'bar' => 'barbar',
          ],
          Vector {'I should feel bad for doing this', 'But yolo'},
          dict[
            1 => 'gross array behavior',
          ],
          Set {'bloop'},
        ],
        dict[
          'foo' => 'foo',
          'bar' => 'barbar',
          'baz' => vec[1, 2, 3],
          0 => 'I should feel bad for doing this',
          1 => 'gross array behavior',
          'bloop' => 'bloop',
        ],
      ),
      'various KeyedContainer types #2' => tuple(
        vec[
          vec['zero'],
          dict[1 => 'one'],
          dict[2 => 'two'],
          Map {3 => 'three'},
        ],
        dict[
          0 => 'zero',
          1 => 'one',
          2 => 'two',
          3 => 'three',
        ],
      ),
    ];
  }

  <<DataProvider('provideTestFlatten')>>
  public function testFlatten(
    Traversable<KeyedContainer<arraykey, mixed>> $traversables,
    dict<arraykey, mixed> $expected,
  ): void {
    expect(Dict\flatten($traversables))->toEqual($expected);
  }

  public static function provideTestFlip(): varray<mixed> {
    return varray[
      tuple(
        darray[],
        dict[],
      ),
      tuple(
        Vector {'the', 'quick', 'brown', 'fox'},
        dict[
          'the' => 0,
          'quick' => 1,
          'brown' => 2,
          'fox' => 3,
        ],
      ),
      tuple(
        Map {
          'foo' => 1,
          'bar' => 'bar',
          'baz' => 0,
          'quz' => 'qux',
        },
        dict[
          1 => 'foo',
          'bar' => 'bar',
          0 => 'baz',
          'qux' => 'quz',
        ],
      ),
    ];
  }

  <<DataProvider('provideTestFlip')>>
  public function testFlip<Tk, Tv as arraykey>(
    KeyedTraversable<Tk, Tv> $traversable,
    dict<Tv, Tk> $expected,
  ): void {
    expect(Dict\flip($traversable))->toEqual($expected);
  }

  public static function provideTestFromKeys(): varray<mixed> {
    return varray[
      tuple(
        Set {},
        $x ==> $x,
        dict[],
      ),
      tuple(
        Map {
          'foo' => 1,
          'bar' => 2,
          'baz' => 3,
        },
        $x ==> $x * $x,
        dict[
          1 => 1,
          2 => 4,
          3 => 9,
        ],
      ),
      tuple(
        varray['the', 'quick', 'brown', 'fox'],
        $s ==> Str\length($s),
        dict[
          'the' => 3,
          'quick' => 5,
          'brown' => 5,
          'fox' => 3,
        ],
      ),
    ];
  }

  <<DataProvider('provideTestFromKeys')>>
  public function testFromKeys<Tk as arraykey, Tv>(
    Traversable<Tk> $keys,
    (function(Tk): Tv) $value_func,
    dict<Tk, Tv> $expected,
  ): void {
    expect(Dict\from_keys($keys, $value_func))->toEqual($expected);
  }

  public static function provideTestFromEntries(): varray<mixed> {
    return varray[
      tuple(
        varray[
          tuple('foo', 1),
          tuple('bar', null),
          tuple('baz', false),
        ],
        dict[
          'foo' => 1,
          'bar' => null,
          'baz' => false,
        ],
      ),
      tuple(
        Vector {
          tuple('foo', 1),
          tuple('bar', null),
          tuple('baz', false),
        },
        dict[
          'foo' => 1,
          'bar' => null,
          'baz' => false,
        ],
      ),
      tuple(
        HackLibTestTraversables::getIterator(varray[
          tuple('foo', 1),
          tuple('bar', null),
          tuple('baz', false),
        ]),
        dict[
          'foo' => 1,
          'bar' => null,
          'baz' => false,
        ],
      ),
    ];
  }

  <<DataProvider('provideTestFromEntries')>>
  public function testFromEntries<Tk as arraykey, Tv>(
    Traversable<(Tk, Tv)> $traversable,
    dict<Tk, Tv> $expected,
  ): void {
    expect(Dict\from_entries($traversable))->toEqual($expected);
  }

  public static function provideTestFromValues(): varray<mixed> {
    return varray[
      tuple(
        darray[],
        $x ==> $x,
        dict[],
      ),
      tuple(
        Vector {'the', 'quick', 'brown', 'fox', 'jumped', 'over', 'the', 'dog'},
        $s ==> Str\length($s),
        dict[
          3 => 'dog',
          5 => 'brown',
          6 => 'jumped',
          4 => 'over',
        ],
      ),
      tuple(
        Map {
          12 => 'the',
          43 => 'brown',
          'hi' => 'fox',
        },
        $x ==> $x,
        dict[
          'the' => 'the',
          'brown' => 'brown',
          'fox' => 'fox',
        ],
      ),
    ];
  }

  <<DataProvider('provideTestFromValues')>>
  public function testFromValues<Tk as arraykey, Tv>(
    Traversable<Tv> $values,
    (function(Tv): Tk) $key_func,
    dict<Tk, Tv> $expected,
  ): void {
    expect(Dict\from_values($values, $key_func))->toEqual($expected);
  }

  public static function provideTestGroupBy(): varray<mixed> {
    return varray[
      tuple(
        varray['the', 'quick', 'brown', 'fox', 'jumped', 'over', 'the', 'dog'],
        $s ==> Str\length($s),
        dict[
          3 => vec['the', 'fox', 'the', 'dog'],
          5 => vec['quick', 'brown'],
          6 => vec['jumped'],
          4 => vec['over'],
        ],
      ),
      tuple(
        HackLibTestTraversables::getIterator(
          varray['the', 'quick', 'brown', 'fox', 'jumped', 'over', 'the', 'dog'],
        ),
        $x ==> Str\length($x) % 2 === 1 ? Str\length($x) : null,
        dict[
          3 => vec['the', 'fox', 'the', 'dog'],
          5 => vec['quick', 'brown'],
        ],
      ),

    ];
  }

  <<DataProvider('provideTestGroupBy')>>
  public function testGroupBy<Tk as arraykey, Tv>(
    Traversable<Tv> $values,
    (function(Tv): ?Tk) $key_func,
    dict<Tk, vec<Tv>> $expected,
  ): void {
    expect(Dict\group_by($values, $key_func))->toEqual($expected);
  }

  public static function provideTestMap(): varray<mixed> {

    $doubler = $x ==> $x * 2;
    return varray[
      // integer vecs
      tuple(darray[], $doubler, dict[]),
      tuple(varray[1], $doubler, dict[0 => 2]),
      tuple(
        Vec\range(10, 1000),
        $doubler,
        dict(Vec\range(20, 2000, 2)),
      ),

      // string vecs
      tuple(varray['a'], $x ==> $x. ' buzz', dict[0 => 'a buzz']),
      tuple(
        varray['a', 'bee', 'a bee'],
        $x ==> $x. ' buzz',
        dict(varray['a buzz', 'bee buzz', 'a bee buzz'])
      ),

      // non-vec: Hack Collections and Hack Arrays
      tuple(
        dict(darray[
          'donald' => 'duck',
          'daffy' => 'duck',
          'mickey' => 'mouse',
        ]),
        $x ==> $x,
        dict['donald' => 'duck', 'daffy' => 'duck', 'mickey' => 'mouse'],
      ),

      tuple(
        Map {'donald' => 'duck', 'daffy' => 'duck', 'mickey' => 'mouse'},
        $x ==> $x,
        dict['donald' => 'duck', 'daffy' => 'duck', 'mickey' => 'mouse'],
      ),

      tuple(
        Vector {10, 20},
        $x ==> $x * 2,
        dict(varray[20, 40]),
      ),

      tuple(
        Set {10, 20},
        $x ==> $x * 2,
        dict[10 => 20, 20 => 40],
      ),

      tuple(
        keyset[10, 20],
        $x ==> $x * 2,
        dict[10 => 20, 20 => 40],
      ),

      tuple(
        HackLibTestTraversables::getIterator(varray[1, 2, 3]),
        $x ==> $x * 2,
        dict(varray[2, 4, 6]),
      ),
      tuple(
        HackLibTestTraversables::getKeyedIterator(darray[10 => 1, 20 => 2, 30 => 3]),
        $x ==> $x * 2,
        dict[10 => 2, 20 => 4, 30 => 6],
      ),
    ];
  }

  <<DataProvider('provideTestMap')>>
  public function testMap<Tk as arraykey, Tv1, Tv2>(
    KeyedTraversable<Tk, Tv1> $traversable,
    (function (Tv1): Tv2) $func,
    dict<Tk, Tv2> $expected,
  ): void {
    expect(Dict\map($traversable, $func))->toEqual($expected);
  }

  public static function provideTestMapKeys(): varray<mixed> {
    return varray[
      tuple(
        dict[
          'the' => 'the',
          'quick' => 'quick',
          'brown' => 'brown',
          'fox' => 'fox',
          'jumps' => 'jumps',
          'over' => 'over',
          'lazy' => 'lazy',
          'dog' => 'dog',
        ],
        $s ==> Str\length($s),
        dict[
          3 => 'dog',
          5 => 'jumps',
          4 => 'lazy',
        ],
      ),
      tuple(
        Vector {'the', 'quick', 'brown', 'fox'},
        $k ==> (string)$k,
        dict[
          '0' => 'the',
          '1' => 'quick',
          '2' => 'brown',
          '3' => 'fox',
        ],
      ),
      tuple(
        HackLibTestTraversables::getKeyedIterator(darray[
          'foo' => 'foo',
          'bar' => 'bar',
          'baz' => 'baz',
        ]),
        $s ==> Str\reverse($s),
        dict[
          'oof' => 'foo',
          'rab' => 'bar',
          'zab' => 'baz',
        ],
      ),
    ];
  }

  <<DataProvider('provideTestMapKeys')>>
  public function testMapKeys<Tk1, Tk2 as arraykey, Tv>(
    KeyedTraversable<Tk1, Tv> $traversable,
    (function(Tk1): Tk2) $key_func,
    dict<Tk2, Tv> $expected,
  ): void {
    expect(Dict\map_keys($traversable, $key_func))->toEqual($expected);
  }

  public static function provideTestMapWithKey(): varray<mixed> {
    return varray[
      tuple(
        darray[],
        ($a, $b) ==> null,
        dict[],
      ),
      tuple(
        Vector {'the', 'quick', 'brown', 'fox'},
        ($k, $v) ==> (string)$k.$v,
        dict[
          0 => '0the',
          1 => '1quick',
          2 => '2brown',
          3 => '3fox',
        ],
      ),
      tuple(
        HackLibTestTraversables::getKeyedIterator(Vec\range(1, 5)),
        ($k, $v) ==> $k * $v,
        dict[
          0 => 0,
          1 => 2,
          2 => 6,
          3 => 12,
          4 => 20,
        ],
      ),
    ];
  }

  <<DataProvider('provideTestMapWithKey')>>
  public function testMapWithKey<Tk as arraykey, Tv1, Tv2>(
    KeyedTraversable<Tk, Tv1> $traversable,
    (function(Tk, Tv1): Tv2) $value_func,
    dict<Tk, Tv2> $expected,
  ): void {
    expect(Dict\map_with_key($traversable, $value_func))->toEqual($expected);
  }

  public static function provideTestPull(): varray<mixed> {
    return varray[
      tuple(
        Vector {'the', 'quick', 'brown', 'fox'},
        $x ==> $x,
        $s ==> Str\length($s),
        dict[
          3 => 'fox',
          5 => 'brown',
        ],
      ),
      tuple(
        HackLibTestTraversables::getIterator(
          varray[1, 3, 5, 7, 9],
        ),
        ($v) ==> $v * $v,
        $k ==> (string)$k,
        dict[
          '1' => 1,
          '3' => 9,
          '5' => 25,
          '7' => 49,
          '9' => 81,
        ],
      ),
    ];
  }

  <<DataProvider('provideTestPull')>>
  public function testPull<Tk as arraykey, Tv1, Tv2>(
    Traversable<Tv1> $traversable,
    (function(Tv1): Tv2) $value_func,
    (function(Tv1): Tk) $key_func,
    dict<Tk, Tv2> $expected,
  ): void {
    expect(Dict\pull($traversable, $value_func, $key_func))
      ->toEqual($expected);
  }

  public static function provideTestPullWithKey(): varray<mixed> {
    return varray[
      tuple(
        Vector {'the', 'quick', 'brown', 'fox'},
        ($k, $v) ==> $k,
        ($k, $v) ==> Str\slice($v, $k),
        dict[
          'the' => 0,
          'uick' => 1,
          'own' => 2,
          '' => 3,
        ],
      ),
      tuple(
        darray[10 => 'foo', 20 => 'food', 30 => 'fool', 40 => 'rude'],
        ($k, $v) ==> $v.(string)$k,
        ($k, $v) ==> Str\slice($v, 0, 3),
        dict[
          'foo' => 'fool30',
          'rud' => 'rude40',
        ],
      ),
    ];
  }

  <<DataProvider('provideTestPullWithKey')>>
  public function testPullWithKey<Tk1, Tk2 as arraykey, Tv1, Tv2>(
    KeyedTraversable<Tk1, Tv1> $traversable,
    (function(Tk1, Tv1): Tv2) $value_func,
    (function(Tk1, Tv1): Tk2) $key_func,
    dict<Tk2, Tv2> $expected,
  ): void {
    expect(Dict\pull_with_key($traversable, $value_func, $key_func))
      ->toEqual($expected);
  }
}
