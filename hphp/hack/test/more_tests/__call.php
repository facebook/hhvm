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

class Foo {
  public function __call(string $name, array $args): string {
    return $name;
  }

  public static function __callStatic(string $name, array $args): int {
    return 123;
  }
}

class FooTwo extends Foo {
  public function someFunc(int $hello): int {
    return $hello;
  }

  static public function someStaticFunc(string $duck): string {
    return $duck;
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

function call_static_test1(): int {
  return Foo::batman();
}

function call_static_test2(): int {
  return FooTwo::batman();
}

function call_static_test3(): string {
  return FooTwo::someStaticFunc('hello');
}

function direct_call(): string {
  return (new Foo())->call('batman', array());
}

function direct_static_call(): int {
  return Foo::callStatic('batman', array());
}
