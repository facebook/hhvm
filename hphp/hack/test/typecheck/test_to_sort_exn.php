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

class A extends \Exception {
  public function f(): void {}
}

function test(): void {
  try {
  } catch (A $e) {
    $e->f();
  }
}
