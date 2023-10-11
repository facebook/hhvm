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

class Base {
  public function foo(): this {
    return $this;
  }
}

class Child extends Base {
  public function foo(): this {
    return parent::foo();
  }
}
