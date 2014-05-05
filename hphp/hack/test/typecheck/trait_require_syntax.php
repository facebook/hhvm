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

interface I1 {
  const int ICONST = 1;

  public function baz(): void;
}

interface I2 extends I1 {}

class Super {
  protected function foo(): void {
    echo __METHOD__, "\n";
  }
}

trait Trait1 {
  require extends Super;
  require implements I1;

  public function bar(): void {
    echo '[', self::ICONST, ']', "\n";
    $this->foo();
    $this->baz();
  }

  // <<Override>> // FIXME: check without a use class?
  // protected function override_me(): void {}
}

trait Trait2 {
  use Trait1; // Requirements of Trait1 inherited

  protected function f() {
    return $this->foo();
  }
  protected function g() {
    return $this->baz();
  }
}

class C
 extends Super
 implements I2
{
  use Trait2;

  public function baz(): void {
    echo "baz", "\n";
  }
}
