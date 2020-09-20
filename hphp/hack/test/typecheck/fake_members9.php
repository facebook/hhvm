<?hh // strict
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

function show<T>(T $x): void {}

interface I { }
class A implements I { }
class B implements I { }

class Z {
  public static async function genNotFoo(): Awaitable<void> {
    return;
  }
}

class Unrelated {
  public I $foo;

  public function __construct(I $foo) {
    $this->foo = $foo;
  }

  public async function genFooAsA(): Awaitable<A> {
    if (!($this->foo is A)) {
      $this->foo = new A();
    }
    await Z::genNotFoo();
    return $this->foo;
  }
}
