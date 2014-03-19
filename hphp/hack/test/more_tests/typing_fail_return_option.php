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


bool function is_null(Option<T> $x) {
  return true;
}

Option<int> function test() {
  return 0;
}

class A {
  Option<A> $x;

  public void function setX(A $x) {
    $this->x = $x;
  }

  public A function getX(dyn $obj) {
    $y = (!($obj instanceof A) || $obj->setX(new A()));
    $y = (($obj instanceof A) && $obj->setX(new A()));
    if (($x = $this->x) && ($x instanceof A)) {
      return $x;
    }
    return $this;
  }
}
