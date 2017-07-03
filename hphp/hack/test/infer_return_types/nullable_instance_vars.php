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
  private $a;
  private $b = null;
  private $c;
  private $d = null;
  private $e = 1;
  private $f = 1;

  private static $s;
  private static $t = null;
  private static $u = 1;

  public function __construct() {
    $this->a = 1;
    $this->b = 1;
    $this->e = 1;

    self::$s = 1;
    self::$t = 1;
    self::$u = 1;
  }

  public function foo(): void {
    $this->c = 1;
    $this->d = 1;
    $this->f = 1;
  }
}
