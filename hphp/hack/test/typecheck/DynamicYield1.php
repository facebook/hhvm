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

function prep<T>(Awaitable<T> $awaitable): T {
  // UNSAFE
}

class A {}
class B extends A {}

class Foo {
  use DynamicYield;

  public async function yieldA(): Awaitable<A> {
    return new A();
  }

  public async function yieldB(): Awaitable<B> {
    return new B();
  }

  protected async function yieldSomeInt(): Awaitable<int> {
    return 123;
  }

  private async function yieldSomeString(): Awaitable<string> {
    return 'hello';
  }

  public function someInt(): int {
    return prep($this->genSomeInt());
  }

  public function someString(): string {
    return prep($this->genSomeString());
  }

  public function getAnotherString(): string {
    return 'howdy';
  }

  /**
   * Override with something else
   */
  public function getA(): int {
    return 123;
  }
}

class Bar extends Foo {
  public function anotherInt(): int {
    return prep($this->genSomeInt());
  }
}

function bar(): Awaitable<A> {
  return (new Foo())->genA();
}

function buck(): int {
  return (new Foo())->getA();
}

function goose(): Awaitable<string> {
  return (new Foo())->genAnotherString();
}
