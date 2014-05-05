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
  private int $bar;
  private string $duck;
  public function __construct() {
    $this->setBar(123);
    $this->duck = 'hello';
  }

  private function setBar(int $bar) {
    $this->bar = $bar;
    return 123; // This works without this line
  }
}

