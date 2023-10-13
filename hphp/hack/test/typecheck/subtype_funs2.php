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

class Foo {
  private (function(): int) $x;
  public function __construct((function(): mixed) $x) {
    $this->x = $x;
  }
}
