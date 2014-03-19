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

function foo(int $parent): int {
  $a = "hello $parent";
  $b = <<<PANTS
$parent
PANTS;
  return $parent;
}

class B {
  protected int $parent = 123;

  public function foo(): int {
    $a = "hello $this->parent";
    $b = <<<DUCK
$this->parent
DUCK;
    return $this->parent;
  }
}

class C {
  static public $parent = 123;
  public function foo(): int {
    return C::$parent + self::$parent + static::$parent;
  }
}
