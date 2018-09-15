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
  protected static function f1(string $s): int {
    return 1;
  }

  public static function f2() {
    return class_meth('A', 'f1');
  }

  public static function f3() {
    return class_meth(A::class, 'f1');
  }
}

function foo(): int {
  $p = A::f2();
  return $p('moo');
}

function bar(): int {
  $p = A::f3();
  return $p('moo');
}
