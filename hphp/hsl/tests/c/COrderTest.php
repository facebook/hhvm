<?hh // strict

/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the hphp/hsl/ subdirectory of this source tree.
 *
 */

use namespace HH\Lib\{C, Str, Vec};
use function HH\__Private\MiniTest\expect;
use type HH\__Private\MiniTest\{DataProvider, HackTest};

final class COrderTest extends HackTest {

  const type TCubeDimentions =
    shape('height' => int, 'width' => int, 'depth' => int);

  public static function provideSortableVecs(): vec<(vec<mixed>, bool)> {
    return vec[
      // Empty containers are sorted
      tuple(vec[], true),
      // Containers with one element are sorted
      tuple(vec[0], true),
      // Containers with two ascending elements are sorted
      tuple(vec[0, 1], true),
      // Containers with two descending elements are not sorted
      tuple(vec[1, 0], false),
      // Containers with two strictly equal elements are sorted
      tuple(vec[100, 100], true),
      // Containers with two weakly equal elements are sorted
      tuple(vec[100., 100], true),
      // Containers with n sorted elements are sorted
      tuple(Vec\range(100, 1000, 11), true),
      // Containers with n reversed elements are not sorted
      tuple(Vec\range(1000, 100, 11), false),
      // Containers with alphabetically sorted strings are sorted
      tuple(vec['aaa', 'bbb', 'c', 'ca', 'ccc'], true),
      // Containers with lexiographically sorted strings are not sorted
      tuple(vec['a', 'b', 'aa', 'bb', 'aaa', 'bbb'], false),
    ];
  }

  <<DataProvider('provideSortableVecs')>>
  public function testSortingWithoutComparator(
    vec<mixed> $t,
    bool $expect,
  ): void {
    expect(C\is_sorted($t))->toEqual($expect);
  }

  public static function provideSortableTraversables(
  ): vec<(vec<Traversable<mixed>>, bool)> {
    return Vec\map(
      self::provideSortableVecs(),
      $tuple ==> tuple(self::vecToAllTraversableTypes($tuple[0]), $tuple[1]),
    );
  }

  <<DataProvider('provideSortableTraversables')>>
  public function testSortingTraversablesWithoutComparator(
    vec<Traversable<mixed>> $ts,
    bool $expect,
  ): void {
    foreach ($ts as $t) {
      expect(C\is_sorted($t))->toEqual(
        $expect,
        'Sorting failed for a Traversable of type %s',
        is_object($t)
          ? get_class($t)
          : gettype($t),
      );
    }
  }

  public static function provideNonSortableVecs(
  ): vec<(vec<string>, (function(string, string): int), bool)> {
    $str_len_cmp = (string $a, string $b) ==> Str\length($a) <=> Str\length($b);
    return vec[
      tuple(vec['short', 'longer', 'longest'], $str_len_cmp, true),
      tuple(vec['short', 'tiny', 'longest'], $str_len_cmp, false),
    ];
  }

  <<DataProvider('provideNonSortableVecs')>>
  public function testSortingWithComparator(
    vec<string> $t,
    (function(string, string): int) $comparator,
    bool $expect,
  ): void {
    expect(C\is_sorted($t, $comparator))->toEqual($expect);
  }

  public static function provideNonSortableTraversables(
  ): vec<(vec<Traversable<string>>, (function(string, string): int), bool)> {
    return Vec\map(
      self::provideNonSortableVecs(),
      $tuple ==>
        tuple(self::vecToAllTraversableTypes($tuple[0]), $tuple[1], $tuple[2]),
    );
  }

  <<DataProvider('provideNonSortableTraversables')>>
  public function testSortingTraversablesWithComparator(
    vec<Traversable<string>> $ts,
    (function(string, string): int) $cmp,
    bool $expect,
  ): void {
    foreach ($ts as $t) {
      expect(C\is_sorted($t, $cmp))->toEqual(
        $expect,
        'Sorting failed for a Traversable of type %s',
        is_object($t)
          ? get_class($t)
          : gettype($t),
      );
    }
  }

  public static function provideSortableVecsOfCubes(
  ): vec<(vec<self::TCubeDimentions>, bool)> {
    $make_cube = (int $h, int $w, int $d) ==>
      shape('height' => $h, 'width' => $w, 'depth' => $d);

    return vec[
      tuple(vec[], true),
      tuple(vec[$make_cube(0, 0, 0)], true),
      tuple(vec[$make_cube(10, 10, 10), $make_cube(11, 11, 11)], true),
      tuple(vec[$make_cube(100, 1, 1), $make_cube(10, 10, 10)], true),
      tuple(
        vec[$make_cube(100, 1, 1), $make_cube(10, 10, 10), $make_cube(0, 0, 0)],
        false,
      ),
    ];
  }

  <<DataProvider('provideSortableVecsOfCubes')>>
  public function testSortingVecsOfCubes(
    vec<self::TCubeDimentions> $cubes,
    bool $expect,
  ): void {
    $cube_to_volume = (self::TCubeDimentions $cube) ==>
      $cube['height'] * $cube['width'] * $cube['depth'];
    expect(C\is_sorted_by($cubes, $cube_to_volume))->toEqual($expect);
  }

  public static function provideSortableTraversablesOfCubes(
  ): vec<(vec<Traversable<self::TCubeDimentions>>, bool)> {
    return Vec\map(
      self::provideSortableVecsOfCubes(),
      $tuple ==> tuple(self::vecToAllTraversableTypes($tuple[0]), $tuple[1]),
    );
  }

  <<DataProvider('provideSortableTraversablesOfCubes')>>
  public function testSortingTraversablesOfCubes(
    vec<Traversable<self::TCubeDimentions>> $cubes_traversable,
    bool $expect,
  ): void {
    $cube_to_volume = (self::TCubeDimentions $cube) ==>
      $cube['height'] * $cube['width'] * $cube['depth'];
    foreach ($cubes_traversable as $cubes) {
      expect(C\is_sorted_by($cubes, $cube_to_volume))->toEqual(
        $expect,
        'Sorting failed for a Traversable of type %s',
        is_object($cubes)
          ? get_class($cubes)
          : gettype($cubes),
      );
    }
  }

  public function testIsSortedByUsesComparatorWhenProvided(): void {
    expect(
      C\is_sorted_by(vec[0, 1], $x ==> $x, (int $a, int $b) ==> -($a <=> $b)),
    )->toBeFalse();
  }

  private static function vecToAllTraversableTypes<Tv>(
    vec<Tv> $vec,
  ): vec<Traversable<Tv>> {
    $traversable_to_generator = (vec<Tv> $traversable) ==> {
      foreach ($traversable as $value) {
        yield $value;
      }
    };

    return vec[
      $vec,
      $traversable_to_generator($vec),
      HH\array_mark_legacy(varray($vec)),
      HH\array_mark_legacy(darray($vec)),
      dict($vec),
      new Vector($vec),
      new ImmVector($vec),
      new Map($vec),
      new ImmMap($vec),
    ];
  }
}
