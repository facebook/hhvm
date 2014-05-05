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

class Other {
  // public function __toString(): string {
  //  return "hello";
  // }
}

class Meh {

  public Other $blah;

  public function __construct() {
    $this->blah = new Other();
  }

  public function __toString(): string {
     return "hello";
  }

  public function foo() {
    $b = array("blah" => new Other());
    print "$b[blah]$this";
  }
}
