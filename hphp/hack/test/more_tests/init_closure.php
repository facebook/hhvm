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

class Bar {
  public static function hey((function(): Bar) $cb): Bar {
    return $cb();
  }
}

class Foo {
  private Bar $a, $b;
  public function __construct() {
    $this->a = Bar::hey(function() { return new Bar(); });
    $this->b = Bar::hey(function() { return new Bar(); });
  }
}
