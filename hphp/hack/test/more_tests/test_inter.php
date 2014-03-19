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

interface I {}
class A implements I {}
class B implements I {}

function inter_switch(): I {
  switch(9) {
  case 0: 
    $x = new A();
    break;
  case 1:
    $x = new B();
    break;
  default:
    throw new Exception('bad');
  }
  return $x;
}
