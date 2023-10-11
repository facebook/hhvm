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
abstract class A<T> {

  public function foo(): void {}

  public static function bar(): void {
    $x = new static();
    $x->xxx();
  }

  public static function baz(): A<int> {
    return new static(); // Error - A<T> isn't subtype of A<int>
  }
}
