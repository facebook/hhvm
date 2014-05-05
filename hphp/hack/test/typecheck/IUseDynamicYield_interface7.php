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

interface IBar extends IUseDynamicYield {
  public function yieldStuff(): Awaitable<int>;
}

interface IFoo extends IBar {
  public function yieldMoreStuff(): Awaitable<bool>;
  public function getSomethingElse(): string;
}

class Foo implements IFoo {
  use DynamicYield;

  public async function yieldStuff(): Awaitable<int> {
    return 42;
  }

  public async function yieldMoreStuff(): Awaitable<bool> {
    return false;
  }

  public function getSomethingElse(): string {
    return 'llama';
  }
}

function get_ifoo(): IFoo {
  return new Foo();
}

function test1(): int {
  $ifoo = get_ifoo();
  return $ifoo->getStuff();
}

function test2(): Awaitable<bool> {
  $ifoo = get_ifoo();
  return $ifoo->genMoreStuff();
}

function test3(): string {
  $ifoo = get_ifoo();
  return $ifoo->getSomethingElse();
}
