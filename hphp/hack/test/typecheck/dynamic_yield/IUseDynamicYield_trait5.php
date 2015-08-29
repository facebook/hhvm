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
  public function __call(string $name, array<mixed> $args = array()): void {}
}

interface IUseDynamicYield {}

trait TFoo implements IUseDynamicYield {
  abstract public function genOtherStuff(): Awaitable<int>;

  public async function genStuff(): Awaitable<bool> {
    $other_stuff = await $this->genOtherStuff();
    return true;
  }
}

trait TBar implements IUseDynamicYield {
  public async function genOtherStuff(): Awaitable<int> {
    return 42;
  }

  public async function genSomething(): Awaitable<bool> {
    return true;
  }
}

class Foo {
  use DynamicYield;
  use TFoo;
  use TBar;

  public async function genEvenMoreStuff(): Awaitable<string> {
    $stuff = await $this->genStuff();
    $other_stuff = await $this->genOtherStuff();
    $something = await $this->genSomething();
    return 'llama';
  }
}
