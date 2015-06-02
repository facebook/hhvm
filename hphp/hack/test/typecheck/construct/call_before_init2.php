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

class Foo {
  private int $x;

  public function __construct() {
    $my_class = get_class($this, $this->foo2());
    $this->x = 0;
  }

  public function foo2(): int {
    return 0;
  }
}
