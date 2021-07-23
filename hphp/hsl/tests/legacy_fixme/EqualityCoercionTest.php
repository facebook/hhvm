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

final class EqualityCoercionTest extends HackTest {
  use CoercionsTestUtils;

  <<DataProvider('getData')>>
  public function testeq(mixed $lhs, mixed $rhs): void {
    $this->baseTest($lhs, $rhs, Legacy_FIXME\eq<>);
  }
  <<DataProvider('getData')>>
  public function testneq(mixed $lhs, mixed $rhs): void {
    $this->baseTest($lhs, $rhs, Legacy_FIXME\neq<>);
  }
}
