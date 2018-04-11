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

class A {
  public int $bar;

  public function __construct() {
    $this->bar = 0;
  }

  public function foo(): this {
    return $this;
  }
}

function test(): int {
  return (new A())->foo()->bar;
}
