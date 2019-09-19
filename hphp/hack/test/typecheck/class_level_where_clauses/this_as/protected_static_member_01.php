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

class C {
  private static function me(): void {}
  protected static function accessible(): void {}
}

trait T where this as C {
  public function foo(): void {
    self::accessible();
    self::me();    // Should be error! But self::private is not checked....
  }
}

trait T2 {
  require extends C;

  public function foo(): void {
    self::accessible();
    self::me();    // Same error as in T.
  }
}
