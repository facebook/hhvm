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

abstract class C {
  protected $x;

  public function __construct() {
    $this->x = static::getX();
    if (true) {
      $this->x = 1;
    }
  }

  abstract public static function getX();
}

abstract class D {
  protected $x;

  public function __construct() {
    $this->x = $this->getX();
    if (true) {
      $this->x = 1;
    }
  }

  abstract private function getX();
}

function f(): int { return 1; }

abstract class E {
  protected $x;

  public function __construct() {
    $this->x = f();
    if (true) {
      $this->x = 1;
    }
  }
}

abstract class F {
  protected static $x;

  public function __construct() {
    self::$x = static::getX();
    if (true) {
      self::$x = 1;
    }
  }

  abstract public static function getX();
}

abstract class G {
  protected static $x;

  public function __construct() {
    self::$x = $this->getX();
    if (true) {
      self::$x = 1;
    }
  }

  abstract private function getX();
}
