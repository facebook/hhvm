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
  public static function f1(): int {
    return 1;
  }
}

function foo(): int {
  $p = class_meth('A', 'f1');
  return $p('moo');
}

function bar(): int {
  $p = class_meth(A::class, 'f1');
  return $p('moo');
}
