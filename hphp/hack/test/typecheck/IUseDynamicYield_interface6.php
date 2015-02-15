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

interface IUseDynamicYield {}

interface IFoo extends IUseDynamicYield {
  public function genStuff(): Awaitable<int>;
  public function genMoreStuff(): Awaitable<bool>;
}

class Foo implements IFoo {
  use DynamicYield;
  public async function genStuff(): Awaitable<int> {
    return 42;
  }
}

function get_ifoo(): IFoo {
  return new Foo();
}

function test(): Awaitable<int> {
  $ifoo = get_ifoo();
  return $ifoo->genStuff();
}
