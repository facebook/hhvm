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

trait DynamicYield {
  public function __call(string $name, array $args = array()) {
  }
}

class Foo {
  use DynamicYield;

  public async function yieldSomeString(): Awaitable<string> {
    return 'hello';
  }
}

class Bar extends Foo {
  /** This will work at runtime, but we're disallowing it. It's silly to
    * "override" a function with a different type
    */
  public function getSomeString(): int {
    return 123;
  }

  public function anotherString(): int {
    return $this->getSomeString();
  }
}
