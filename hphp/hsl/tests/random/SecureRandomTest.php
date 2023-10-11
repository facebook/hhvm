<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the hphp/hsl/ subdirectory of this source tree.
 *
 */


use namespace HH\Lib\SecureRandom;
use type HH\__Private\MiniTest\HackTest;

final class SecureRandomTest extends HackTest {
  use RandomTestTrait;

  public function getRandomFloat(): float {
    return SecureRandom\float();
  }

  public function getRandomInt(int $min, int $max): int {
    return SecureRandom\int($min, $max);
  }

  public function getRandomString(
    int $length,
    ?string $alphabet = null,
  ): string {
    return SecureRandom\string($length, $alphabet);
  }
}
