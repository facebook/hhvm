<?hh // strict
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

class Foo {

  public function bar(): void {
  }

}

function f(Vector<Foo> $v): void {
  foreach ($v as $foo) {
    $foo->bar();
    $x = $foo + 5;
  }
}
