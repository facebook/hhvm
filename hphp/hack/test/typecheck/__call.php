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

class Foo {
  public function __call(string $name, array $args): string {
    return $name;
  }
}

class FooTwo extends Foo {
  public function someFunc(int $hello): int {
    return $hello;
  }
}

function call_test1(): string {
  return (new Foo())->batman();
}

function call_test2(): string {
  return (new FooTwo())->batman();
}

function call_test3(): int {
  return (new FooTwo())->someFunc(123);
}

function direct_call(): string {
  return (new Foo())->call('batman', varray[]);
}
