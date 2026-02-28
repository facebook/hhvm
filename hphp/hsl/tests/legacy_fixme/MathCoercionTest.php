<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the hphp/hsl/ subdirectory of this source tree.
 *
 */

use namespace HH\Lib\Legacy_FIXME;
use function HH\__Private\MiniTest\expect;
use type HH\__Private\MiniTest\HackTest;

final class MathCoercionTest extends HackTest {
  public function testArithmetic(): void {
    expect(
      Legacy_FIXME\cast_for_arithmetic(null) +
        Legacy_FIXME\cast_for_arithmetic(false),
    )->toEqual(0);
    expect(
      Legacy_FIXME\cast_for_arithmetic(true) +
        Legacy_FIXME\cast_for_arithmetic('bananas'),
    )->toEqual(1);
    expect(
      (
        Legacy_FIXME\cast_for_arithmetic('12') *
        Legacy_FIXME\cast_for_arithmetic('2.3')
      ) as float,
    )->toEqualWithDelta(27.6, .01);
    expect(
      (
        Legacy_FIXME\cast_for_arithmetic('12ab') -
        Legacy_FIXME\cast_for_arithmetic('2.5foo')
      ) as float,
    )->toEqualWithDelta(9.5, .01);
    expect(
      (
        Legacy_FIXME\cast_for_arithmetic(HH\stdin()) /
        Legacy_FIXME\cast_for_arithmetic(4)
      ) as float,
    )->toEqualWithDelta(0.25, .01);
  }

  public function testExponent(): void {
    expect(Legacy_FIXME\cast_for_exponent(null) ** 2)->toEqual(0);
    expect(Legacy_FIXME\cast_for_exponent(false) ** 2)->toEqual(0);
    expect(Legacy_FIXME\cast_for_exponent(true) ** 2)->toEqual(1);
    expect(Legacy_FIXME\cast_for_exponent('bananas') ** 2)->toEqual(0);
    expect(Legacy_FIXME\cast_for_exponent('12') ** 2)->toEqual(144);
    expect((Legacy_FIXME\cast_for_exponent('2.3') ** 2) as float)
      ->toEqualWithDelta(5.29, .01);
    expect(Legacy_FIXME\cast_for_exponent('12ab') ** 2)->toEqual(144);
    expect((Legacy_FIXME\cast_for_exponent('2.5foo') ** 2) as float)
      ->toEqualWithDelta(6.25, .01);
    expect(Legacy_FIXME\cast_for_exponent(HH\stdout()) ** 2)->toEqual(4);
    expect(Legacy_FIXME\cast_for_exponent(vec['bananas']) ** 2)->toEqual(0);
    expect(
      Legacy_FIXME\cast_for_exponent(Legacy_FIXME\cast_for_exponent<>) ** 2,
    )->toEqual(0);
    expect(Legacy_FIXME\cast_for_exponent(4) ** 2)->toEqual(16);
  }
}
