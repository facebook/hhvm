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

trait A {
  protected int $x;
}

trait C {
  use A;
}

abstract class B {
  use C;
  
  public function __construct() {

  }
}

class D extends B {
  
}
