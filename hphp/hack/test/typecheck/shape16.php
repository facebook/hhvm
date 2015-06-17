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

class A {}
class B extends A {}
class C extends A {}

type myshape = shape('field' => A);

function test(): myshape {
  if (true) {
    $x = shape('field' => new B());
  } else {
    $x = shape('field' => new C());
  }
  return $x;
}
