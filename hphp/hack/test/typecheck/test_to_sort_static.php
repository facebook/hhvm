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
  public static int $x = 0;
  public static function f(bool $x): int {
    return 0;
  }
}

function test(): void {
  A::$x;
  A::f(true);
}

class B<T> {
  public static function f(T $x): T {
    return $x;
  }
}

function test2(): int {
  return B::f(0);
}
