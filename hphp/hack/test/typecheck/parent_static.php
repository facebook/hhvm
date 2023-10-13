<?hh
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
  public function f1(): void {}
}

class B extends A {
  public static function f2(): void {
    parent::f1();
  }
}
