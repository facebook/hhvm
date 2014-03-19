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

interface A {}

// until we find something better
class Null {
  const ?A A = null;
}

class B implements A {}

class C implements A {}

class D {
  protected Map<string, A> $x;

  public function __construct() {
    $this->x = Map{};
  }

  public function foo() {
    $y = Null::A;
    if (true) {
      $y = new B();
    } else {
      $y = new C();
    }
    if($y) {
      $this->x['meh'] = $y;
    }
  }
}
