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

trait MyTrait {
  protected ?int $x;
  public function getInt(): int {
    if ($this->x === null) {
      return 0;
    } else {
      return $this->x;
    }
  }
}

class C {
  use MyTrait;
  protected string $x;

  public function __construct() {
    $this->x = 'hello world';
  }
}

function takes_int(int $i): void {}

function broken(): void {
  $c = new C();
  takes_int($c->getInt());
}

// broken();
