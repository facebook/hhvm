<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the hphp/hsl/ subdirectory of this source tree.
 *
 */

use function HH\__Private\MiniTest\expect;
use type HH\__Private\MiniTest\HackTest;

final class RefTest extends HackTest {
  public function testRefiness(): void {
    $myref = new \HH\Lib\Ref(0);
    $getsetref = new \HH\Lib\Ref(0);
    // Non-inout arg
    (
      (\HH\Lib\Ref<int> $ref) ==> {
        $ref->value++;
        $getsetref->set($ref->get());
      }
    )($myref);
    expect($myref->value)->toEqual(1);
    expect($getsetref->get())->toEqual(1);
    // implicit capture, which is always byval
    (
      () ==> {
        $myref->value++;
        $getsetref->set($myref->get());
      }
    )();
    expect($myref->value)->toEqual(2);
    expect($getsetref->get())->toEqual(2);
  }
}
