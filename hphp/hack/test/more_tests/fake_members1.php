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

class X {
  private static ?int $x = null;

  public function getX(): int {
    if(self::$x === null) {
      self::$x = 0;
    }
    return self::$x;
  }
}
