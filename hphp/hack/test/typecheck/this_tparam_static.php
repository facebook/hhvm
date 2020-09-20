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

class BadClass {
  private static ?this $instance;

  public static function getInstance(): this {
    $instance = self::$instance;
    if ($instance === null) {
      $instance = new static();
      self::$instance = $instance;
    }
    return $instance;
  }
}

class ChildClass extends BadClass {}

function foo(): ChildClass {
  // Cache BadClass
  BadClass::getInstance();
  // Now returns the wrong class!
  return ChildClass::getInstance();
}
