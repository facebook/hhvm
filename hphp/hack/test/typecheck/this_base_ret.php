<?hh // partial
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
  public function bar(int $id): this {
    // This comes from a real world example, where the if block turned $this
    // into an unresolved type
    if ($id) {
    } else {
    }
    return $this->foo();
  }

  public function duck(): this {
    return $this->foo();
  }
}
