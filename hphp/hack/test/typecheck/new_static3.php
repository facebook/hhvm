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

<<__ConsistentConstruct>>
class A {
  <<__NeedsConcrete>>
  public static function bar(): void {
    $x = new static();
  }
}

class B {
  <<__NeedsConcrete>>
  public static function bar(): void {
    $x = new static();
  }
}
