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

class Toto {
  public static function f1(): float {
    $x = array(5 => 5.0);
    return $x[5];
  }

  public static function f2(): float {
    return (array(5 => 5.0))[5];
  }

  public static function f3(): float {
    $x = Map { 5 => 5.0 };
    return $x[5];
  }

  public static function f4(): float {
    return (Map { 5 => 5.0 })[5];
  }
}
