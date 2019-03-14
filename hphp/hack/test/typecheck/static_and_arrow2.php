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

class Bar {
  public static function f2() {
  }
}
class Foo {
  public function f1() {
    $bar = new Bar();
    $bar->f2();
  }
}
