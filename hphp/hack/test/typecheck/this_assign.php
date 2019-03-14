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

class X {

  private ?int $a;

  private function test(): void {
    $this->a = 0;
    $this->foo($this->a);
  }

  private function foo(int $a): void {}
}
