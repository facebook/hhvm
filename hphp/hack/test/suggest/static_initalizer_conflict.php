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

class C {
  private static $x;

  public static function set(bool $y): void {
    self::$x = $y;
  }
}

class D {
  private static $x = 42;

  public static function set(bool $y): void {
    self::$x = $y;
  }
}
