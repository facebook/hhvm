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

class A {
  private function f1(string $s): int {
    return 1;
  }
}

function foo(A $a): int {
  $p = inst_meth($a, 'f1');
  return $p('moo');
}
