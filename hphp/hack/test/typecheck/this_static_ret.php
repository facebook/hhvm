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

<<__ConsistentConstruct>>
class Foo {
  public static function get(): this {
    return new static();
  }
}

class Bar extends Foo {
  public function something(): Bar {
    return self::get();
  }
}
