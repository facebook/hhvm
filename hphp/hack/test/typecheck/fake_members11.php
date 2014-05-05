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

class Bar1 {
  private ?Vector<int> $foo;

  public function __construct(?Vector<int> $foo) {
    $this->foo = $foo;
  }

  public function f(): void {
    if ($this->foo) {
      random_other_function();
      $this->foo[0] = 0;
    }
  }
}

function random_other_function(): void {
}
