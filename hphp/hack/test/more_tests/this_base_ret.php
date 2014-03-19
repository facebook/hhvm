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
