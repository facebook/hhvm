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

interface IUseDynamicYield {}

trait DynamicYield implements IUseDynamicYield {
  public function __call(string $name, array<mixed> $args = array()): void {
  }
}

trait TFoo {

  require implements IUseDynamicYield;

  public async function genStuff(): Awaitable<bool> {
    return true;
  }
}

class Foo {
  use DynamicYield;
  use TFoo;

  public async function genEvenMoreStuff(): Awaitable<string> {
    $stuff = await $this->genStuff();
    return 'llama';
  }
}
