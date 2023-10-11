<?hh
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

interface Foo {
  public function bar() : void;
}

class Baz implements Foo {
  public static function bar() : void { } // ERROR
  public function bar() : void { }
}
