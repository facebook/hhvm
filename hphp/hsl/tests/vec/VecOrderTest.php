<?hh // strict
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the hphp/hsl/ subdirectory of this source tree.
 *
 */

use namespace HH\Lib\{C, Math, Str, Vec};
use function \HH\__Private\MiniTest\expect;
use type HH\__Private\MiniTest\{DataProvider, HackTest};

final class VecOrderTest extends HackTest {

  public static function provideTestRange(): varray<mixed> {
    return varray[
      tuple(1, 10, null, vec[1, 2, 3, 4, 5, 6, 7, 8, 9, 10]),
      tuple(1, 10, 1, vec[1, 2, 3, 4, 5, 6, 7, 8, 9, 10]),
      tuple(1, 10, 2, vec[1, 3, 5, 7, 9]),
      tuple(1, 10, 3, vec[1, 4, 7, 10]),
      tuple(1, 10, 9, vec[1, 10]),
      tuple(10, 1, null, vec[10, 9, 8, 7, 6, 5, 4, 3, 2, 1]),
      tuple(10, 1, 5, vec[10, 5]),
      tuple(
        1.0,
        2.0,
        0.1,
        vec[1.0, 1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7, 1.8, 1.9, 2.0]
      ),
      tuple(3.0, -1.5, 1.5, vec[3.0, 1.5, 0.0, -1.5]),
      tuple(
        -4294967296.0,
        -4294968548.5,
        125.25,
        vec[
          -4294967296.0,
          -4294967421.25,
          -4294967546.5,
          -4294967671.75,
          -4294967797.0,
          -4294967922.25,
          -4294968047.5,
          -4294968172.75,
          -4294968298.0,
          -4294968423.25,
          -4294968548.5,
        ],
      ),
      tuple(1, 1, 1, vec[1]),
      tuple(1, 1, 2, vec[1]),
      tuple(3.5, 3.5, 1.0, vec[3.5]),
      tuple(1, 10, 11, vec[1]),
      tuple(3.5, 7.5, 6.0, vec[3.5]),
      tuple(-3.5, -7.5, 6.0, vec[-3.5]),
      tuple(-4, 4, 3, vec[-4, -1, 2]),
      tuple(-2, 2, 5, vec[-2]),
      tuple(4, -4, 3, vec[4, 1, -2]),
      tuple(2, -2, 5, vec[2]),
    ];
  }

  <<DataProvider('provideTestRange')>>
  public function testRange<Tv as num>(
    Tv $start,
    Tv $end,
    ?Tv $increment,
    vec<Tv> $expected,
  ): void {
    $actual = Vec\range($start, $end, $increment);
    expect(C\count($actual))->toEqual(C\count($expected));
    for ($i = 0; $i < C\count($actual); $i++) {
      expect((float) $actual[$i])->toAlmostEqual((float) $expected[$i]);
    }
  }

  public static function provideTestRangeException(): varray<mixed> {
    return varray[
      tuple(0, 1, 0),
      tuple(-10, 10, -30),
    ];
  }

  <<DataProvider('provideTestRangeException')>>
  public function testRangeException<Tv as num>(
    Tv $start,
    Tv $end,
    Tv $increment,
  ): void {
    expect(() ==> Vec\range($start, $end, $increment))
      ->toThrow(InvariantException::class);
  }

  public static function provideTestReverse(): varray<mixed> {
    return varray[
      tuple(
        vec[1, 2, 3, 4, 5],
        vec[5, 4, 3, 2, 1],
      ),
      tuple(
        Vector {'the', 'quick', 'brown', 'fox', 'jumped'},
        vec['jumped', 'fox', 'brown', 'quick', 'the'],
      ),
      tuple(
        HackLibTestTraversables::getIterator(Vec\range(1, 5)),
        vec[5, 4, 3, 2, 1],
      ),
    ];
  }

  <<DataProvider('provideTestReverse')>>
  public function testReverse<Tv>(
    Traversable<Tv> $traversable,
    vec<Tv> $expected,
  ): void {
    expect(Vec\reverse($traversable))->toEqual($expected);
  }

  public static function provideTestShuffle(): varray<varray<(function(): Traversable<int>)>> {
    return varray[
      varray[
        () ==> vec[8, 6, 7, 5, 3, 0, 9],
      ],
      varray[
        () ==> vec[0, 1, 2, 4, 5, 6, 7],
      ],
      varray[
        () ==> HackLibTestTraversables::getIterator(varray[8, 6, 7, 5, 3, 0, 9]),
      ],
    ];
  }

  <<DataProvider('provideTestShuffle')>>
  public function testShuffle(
    (function(): Traversable<int>) $input,
  ): void {
    for ($i = 0; $i < 1000; $i++) {
      $shuffled1 = Vec\shuffle($input());
      $shuffled2 = Vec\shuffle($input());
      $vec_input = vec($input());

      expect(Vec\sort($shuffled1))->toEqual(Vec\sort($vec_input));
      expect(Vec\sort($shuffled2))->toEqual(Vec\sort($vec_input));

      // There is a chance that even if we shuffle we get the same thing twice.
      // That is ok but if we try 1000 times we should get different things.
      if (
        $shuffled1 !== $shuffled2 &&
        $shuffled1 !== $vec_input &&
        $shuffled2 !== $vec_input
      ) {
        return;
      }
    }

    self::fail('We shuffled 1000 times and the value never changed');
  }

  public static function provideTestSort(): varray<mixed> {
    return varray[
      tuple(
        vec['the', 'quick', 'brown', 'fox'],
        null,
        vec['brown', 'fox', 'quick', 'the'],
      ),
      tuple(
        vec['the', 'quick', 'brown', 'fox'],
        /* HH_FIXME[4297] The type of the lambda argument(s) could not be inferred */
        ($a, $b) ==> $a[1] <=> $b[1],
        vec['the', 'fox', 'brown', 'quick'],
      ),
      tuple(Vector {1, 1.2, -5.7, -5.8}, null, vec[-5.8, -5.7, 1, 1.2]),
      tuple(
        HackLibTestTraversables::getIterator(varray[8, 6, 7, 5, 3, 0, 9]),
        null,
        vec[0, 3, 5, 6, 7, 8, 9],
      ),
    ];
  }

  <<DataProvider('provideTestSort')>>
  public function testSort<Tv>(
    Traversable<Tv> $traversable,
    ?(function(Tv, Tv): int) $comparator,
    vec<Tv> $expected,
  ): void {
    expect(Vec\sort($traversable, $comparator))->toEqual($expected);
  }

  public static function provideTestSortBy(): varray<mixed> {
    return varray[
      tuple(
        varray['the', 'quick', 'brown', 'fox', 'jumped'],
        $s ==> Str\reverse($s),
        null,
        vec['jumped', 'the', 'quick', 'brown', 'fox'],
      ),
      tuple(
        HackLibTestTraversables::getIterator(
          varray['the', 'quick', 'brown', 'fox', 'jumped'],
        ),
        $s ==> Str\reverse($s),
        null,
        vec['jumped', 'the', 'quick', 'brown', 'fox'],
      ),
      tuple(
        Vector {'the', 'quick', 'brown', 'fox', 'jumped'},
        $s ==> Str\reverse($s),
        ($a, $b) ==> $b <=> $a,
        vec['fox', 'brown', 'quick', 'the', 'jumped'],
      ),
      tuple(
        vec['the', 'quick', 'brown', 'fox', 'jumped'],
        (mixed $_) ==> 0,
        null,
        vec['the', 'quick', 'brown', 'fox', 'jumped'],
      ),
      tuple(
        Vector {'the', 'quick', 'brown', 'fox', 'jumped', 'over'},
        $x ==> Str\length($x),
        null,
        vec['the', 'fox', 'over', 'quick', 'brown', 'jumped'],
      ),
      tuple(
        Vector {'the', 'quick', 'fox', 'jumped', 'over'},
        $x ==> Str\length($x) / 2,
        null,
        vec['the', 'fox', 'over', 'quick', 'jumped'],
      ),
      // If `scalar_func` returns a tuple, it should sort item-by-item in the tuple
      // Sort by string length, and sort by the string itself as a tie breaker
      tuple(
        vec['hello', 'world', 'an', 'awesome', 'test', 'zeuss', 'hacks'],
        (string $x) ==> tuple(Str\length($x), $x),
        null,
        vec['an', 'test', 'hacks', 'hello', 'world', 'zeuss', 'awesome'],
      )
    ];
  }

  <<DataProvider('provideTestSortBy')>>
  public function testSortBy<Tv, Ts>(
    Traversable<Tv> $traversable,
    (function(Tv): Ts) $scalar_func,
    ?(function(Ts, Ts): int) $comparator,
    vec<Tv> $expected,
  ): void {
    expect(Vec\sort_by($traversable, $scalar_func, $comparator))
      ->toEqual($expected);
  }
}
