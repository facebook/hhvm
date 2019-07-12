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

class C {
 public function foo(): this {
   return $this;
 }
}

interface I where this as C {}

function return_self(I $i): C {
  return $i->bar();
}
