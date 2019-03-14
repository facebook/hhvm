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

trait MyTrait {
  protected static function foo(): int {
    return 4;
  }
}

class A {
  use MyTrait;

  public function bar(): int {
    self::foo();
    static::foo();
    return MyTrait::foo();
  }
}
