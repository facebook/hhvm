<?hh // partial
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
  private function f1(string $s): int {
    return 1;
  }
  public function f2() {
    return inst_meth($this, 'f1');
  }
}

function foo(A $a): int {
  $p = $a->f2();
  return $p('moo');
}
