<?hh
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

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
