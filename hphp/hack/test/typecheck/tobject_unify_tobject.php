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

class C<T> {
  public static function foo(T $a1, T $a2) {}
}

function f1() {
  C::foo(new SomethingUnknown(), new SomethingElseUnknown());
}
