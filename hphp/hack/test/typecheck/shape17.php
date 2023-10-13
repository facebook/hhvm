<?hh
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

class A {}
class B extends A {}
class C extends A {}

type myshape = shape('field' => C);

function test(): myshape {
  if (true) {
    $x = shape('field' => new A());
  } else {
    $x = shape('field' => new C());
  }
  return $x;
}
