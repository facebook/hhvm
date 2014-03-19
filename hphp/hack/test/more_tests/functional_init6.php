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

// File testing corner cases for initialization of class members


class A {
  private string $x;

  public function __construct(string $x) {
    $this->setX($x);
    nasty($this);
  }

  private function setX(string $x): void {
    $this->x = $x;
  }
}

function nasty(A $x): void {
}


