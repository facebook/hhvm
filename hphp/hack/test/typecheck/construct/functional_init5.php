<?hh // partial
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

// File testing corner cases for initialization of class members

class A {
  private string $x;

  public function __construct(string $x) {
    if ($x === '') {
      $this->x = 'foo';
    }
    $this->setX($x);
  }

  protected function setX(string $x): void {
    $this->x = $x;
  }
}

// We redefine setX, so that it doesn't initialize the parent class properly
// anymore.
class B extends A {

  public function __construct() {
    parent::__construct('fooled');
  }

  protected function setX(string $x): void {}

}
