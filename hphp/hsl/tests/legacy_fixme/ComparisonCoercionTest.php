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
use type HH\__Private\MiniTest\{HackTest, CoercionsTestUtils, DataProvider};

final class ComparisonCoercionTest extends HackTest {
  use CoercionsTestUtils;

  <<DataProvider('getData')>>
  public function testlt(mixed $lhs, mixed $rhs): void {
    $this->baseTest($lhs, $rhs, Legacy_FIXME\lt<>);
  }
  <<DataProvider('getData')>>
  public function testlte(mixed $lhs, mixed $rhs): void {
    $this->baseTest($lhs, $rhs, Legacy_FIXME\lte<>);
  }
  <<DataProvider('getData')>>
  public function testgt(mixed $lhs, mixed $rhs): void {
    $this->baseTest($lhs, $rhs, Legacy_FIXME\gt<>);
  }
  <<DataProvider('getData')>>
  public function testgte(mixed $lhs, mixed $rhs): void {
    $this->baseTest($lhs, $rhs, Legacy_FIXME\gte<>);
  }
  <<DataProvider('getData')>>
  public function testcmp(mixed $lhs, mixed $rhs): void {
    $this->baseTest($lhs, $rhs, Legacy_FIXME\cmp<>);
  }
}
