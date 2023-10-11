<?hh
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
  public function foo(): T;
}

abstract class AClass<T> implements IFace<T> {}

class BClass extends AClass<bool> {
  public function foo(): int {
    return $this->foo();
  }
}
