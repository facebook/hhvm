<?hh // strict
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
  public function __call(string $name, array<mixed> $args = array()): void {
  }
}

abstract class A {
  use DynamicYield;
  abstract public function yieldFoo(): Awaitable<int>;
}

class B extends A {
  public async function yieldFoo(): Awaitable<int> {
    return 42;
  }
}
