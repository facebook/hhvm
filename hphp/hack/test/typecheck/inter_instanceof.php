<?hh // strict
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

class A {
  public function get(): void {}
}

function test(): void {
  $x = Vector {};
  $x[] = 0;
  $x[] = '';
  $y = $x[0];
  if($y instanceof A) {
    $y->get();
  }
}
