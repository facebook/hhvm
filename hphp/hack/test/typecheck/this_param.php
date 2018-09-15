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
<<__ConsistentConstruct>>
class Foo {
  public static function get(): this {
    return new static();
  }

  public function set(this $x): this {
    return $x;
  }

  public function test(Foo $foo): void {
    $this->set(new static());
    $this->set($foo);
  }
}
