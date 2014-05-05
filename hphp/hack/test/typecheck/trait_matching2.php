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
  abstract protected function lol(): int;

  public function getInt(): int {
    return $this->lol();
  }
}

class C {
  use MyTrait;

  protected function lol(): string {
    return 'hello world';
  }
}

function takes_int(int $i): void {}

function broken(): void {
  $c = new C();
  takes_int($c->getInt());
}

// broken();
