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

function f((A, bool) $x): void {
  // subtyping works with tuples
  $x = tuple(new B(), true);
}
