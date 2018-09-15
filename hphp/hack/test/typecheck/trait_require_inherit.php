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

interface IParent {
  const FOO = 'bar';

  public function whatevs();
}

abstract class ParentClass implements IParent {
  protected function bar() {}
}

abstract class Kid extends ParentClass {
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
