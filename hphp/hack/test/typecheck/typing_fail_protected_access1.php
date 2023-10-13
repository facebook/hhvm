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

class Foo {
  protected function f(): void {}
}

class Bar {
  public function x(): ?int {
    $x = new Foo();
    $x->f(); // ERROR
  }
}
