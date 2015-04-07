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

function show<T>(T $x): void {}

interface I { }
class A implements I { }
class B implements I { }

class Z {
  public static async function genNotFoo(): Awaitable<void> {
    return null;
  }
}

class Unrelated {
  public I $foo;

  public function __construct(I $foo) {
    $this->foo = $foo;
  }

  public async function genFooAsA(): Awaitable<A> {
    if (!($this->foo instanceof A)) {
      $this->foo = new A();
    }
    await Z::genNotFoo();
    return $this->foo;
  }
}
