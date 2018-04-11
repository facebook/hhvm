<?hh // strict
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

<<__ConsistentConstruct>>
class A {
  public static function getInstance(): this {
    static $instance = null;
    if (null === $instance) {
      $instance = new static();
    }
    return $instance;
  }
}
