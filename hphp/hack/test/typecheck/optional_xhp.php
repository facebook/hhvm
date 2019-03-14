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

class A {
  public ?:x:base $xhp;

  public function foo(?:x:base $x): ?:x:base {
    return null;
  }
}

class :x:base {}
