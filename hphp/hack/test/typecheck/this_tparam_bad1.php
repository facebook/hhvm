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

// This test illustrates the danger with instantiating a non-covariant type
// variable with this. The analysis used to reject code like this is performed
// by generating expression dependent types
trait SomeTrait<T> {
  private ?T $x;

  public function set(T $x): this {
    $this->x = $x;
    return $this;
  }

  public function get(): ?T {
    return $this->x;
  }
}

class Foo {
  use SomeTrait<this>;
}

class FooTwo extends Foo {}

function foo(Foo $foo): void {
  // Uh oh, we set the member variable to a Foo inside a Foo2
  $foo->set(new Foo());
}

function bar(FooTwo $foo_two): ?FooTwo {
  foo($foo_two);
  return $foo_two->get();
}
