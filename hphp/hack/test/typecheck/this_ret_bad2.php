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

interface IFoo<+T> {}

class Foo<T> implements IFoo<T> {
  public function __construct(T $x) {}
}

class Bar {
  public function getFooOfBar(): Foo<this> {
    return new Foo($this);
  }

}

function expectFooBar(Foo<Bar> $foo): void {}

function expectIFooBar(IFoo<Bar> $foo): void {}

function expectFooAsBar<T as Bar>(Foo<T> $foo): Foo<T> {
  return $foo;
}

function test(Bar $bar): void {
  // Passing it to an 'as' constrained Bar is fine
  hh_show(expectFooAsBar($bar->getFooOfBar()));

  // Can treat as IFoo<Bar> since T in IFoo is covariant
  expectIFooBar($bar->getFooOfBar());

  // But not to Foo<Bar> since T is invariant
  expectFooBar($bar->getFooOfBar());
}
