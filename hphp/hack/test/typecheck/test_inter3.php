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

interface I {}
class A implements I {}
class B implements I {}

function foo(): Vector<I> {
  $x = Vector { new A(), new B() };
  return $x;
}
