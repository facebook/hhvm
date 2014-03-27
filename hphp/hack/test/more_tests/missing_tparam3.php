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

class A<T> {
  private T $x;
  public function __construct(T $x) { $this->x = $x; }
  public function get(): T {return $this->x;}
}

class B extends A {
}
