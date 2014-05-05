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

class Foo1 {
  public function frob(): void {
  }
}

class Bar1 {
  private ?Foo1 $foo;

  public function __construct(?Foo1 $foo) {
    $this->foo = $foo;
  }

  public function f(): void {
    if ($this->foo) {
      random_other_function();
      $this->foo->frob();
    }
  }
}

function random_other_function(): void {
}
