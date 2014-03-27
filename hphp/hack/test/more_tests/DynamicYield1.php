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
    return $this->getSomeInt();
  }

  public function someString(): string {
    return $this->getSomeString();
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
    return $this->getSomeInt();
  }
}

function bar(): Awaitable<A> {
  return (new Foo())->genA();
}

function duck(): Awaitable<void> {
  return (new Foo())->prepareA();
}

function buck(): int {
  return (new Foo())->getA();
}

function donkey(): A {
  return (new Foo())->getB();
}

function goose(): Awaitable<string> {
  return (new Foo())->genAnotherString();
}
