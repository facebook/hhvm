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

interface IParent {
  const FOO = 'bar';

  public abstract function whatevs();
}

abstract class Parent implements IParent {
  protected function bar() {}
}

abstract class Kid extends Parent {
}

class Grandkid extends Kid {
  use KidTrait;

  public function whatevs() {}
}

trait KidTrait {
  require extends Kid;

  protected function foo() {
    $this->bar().self::FOO.$this->whatevs();
  }
}

trait KidTraitChild {
  use KidTrait;

  protected function another_foo() {
    $this->bar().self::FOO.$this->whatevs();
  }
}
