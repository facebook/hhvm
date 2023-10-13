<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the hphp/hsl/ subdirectory of this source tree.
 *
 */

use namespace HH\Lib\Math;
use function HH\__Private\MiniTest\expect;
use type HH\__Private\MiniTest\HackTest;

final class MathConstantsTest extends HackTest {
  const DELTA = .000001;
  public function testInt64Min(): void {
    expect(Math\INT64_MIN)->toBeLessThan(0);
    $less = Math\INT64_MIN - 1;
    expect($less === Math\INT64_MAX || $less is float)->toBeTrue();
  }
  public function testPi(): void {
    expect(Math\PI)->toEqualWithDelta(M_PI, self::DELTA);
  }
  public function testE(): void {
    expect(Math\E)->toEqualWithDelta(M_E, self::DELTA);
  }

  public function testNAN(): void {
    expect(Math\NAN is float)->toBeTrue('NAN should be a float');
    expect(Math\NAN)->toNotEqual(Math\NAN); // IEEE behavior
    expect(Math\is_nan(Math\NAN))->toBeTrue('is_nan(NAN) should be true');
  }
}
