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

// Allow delayed implementation for abstract classes

interface IFace<T> {
  public function foo(IFace<T> $x): T;
}

trait MyTrait implements IFace<CClass> {
  public function bar(IFace<CClass> $x): void {
    $this->foo($x);
  }
}

abstract class AClass {
  use MyTrait;
}

class CClass extends AClass {
  public function foo(IFace<CClass> $x): CClass {
    return new CClass();
  }
}
