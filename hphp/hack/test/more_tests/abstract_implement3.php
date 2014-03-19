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

// Allow delayed implementation for abstract classes

interface IFace<T> {
  public function foo(): T;
}

trait MyTrait implements IFace<int> {
  public function bar(): void {
    $this->foo();
  }
}

abstract class AClass<T> {
  use MyTrait;
}

class BClass extends AClass<int> {
  public function foo(): int { 
    return $this->foo();
  }
}
