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

trait FooTrait {
  abstract public function abstractFunc(): int;
  final public function doSomething(): int {
    return $this->abstractFunc();
  }
}

abstract class Base {
  final public function abstractFunc(): int {
    return 4;
  }
}

class Child extends Base {
  use FooTrait;
}
