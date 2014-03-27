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

class X<T> {
  public function __construct(private T $data) {
  }
  public function set(T $x): void {
    $this->data = $x;
  }
  public function get(): T {
    return $this->data;
  }
}

function test(): int {
  $obj = new X(0);
  $x = 0;
  $y = 1;
  $z = 2;
  for($i = 0; $i < 3; $i++) {
    $obj->set($y);
    $x = $obj->get();
    $obj->set($z);
    $y = $obj->get();
    $obj->set('hello');
    $z = $obj->get();
  }
  return $x;
}
