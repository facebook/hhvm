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

trait MyTrait {
  public function id(): this {
    return $this;
  }
}

class C {
  use MyTrait;
}

function f(): C {
  return (new C())->id();
}
