<?hh
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

class B {
  public function f1(string $x): int {
    return 1;
  }
}

class A {
  private function f1(string $s): int {
    return 1;
  }
  public function f2(A $x) {
    if(true) {
      $x = new B();
    }
    return inst_meth($x, 'f1');
  }
}

