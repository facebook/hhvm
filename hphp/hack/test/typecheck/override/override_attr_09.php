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

trait ATr {
  <<__Override>> // enforced onto C1
  public static function foo(): void {}
}

class C1 {
  use ATr;
}
