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

class X {
  private ?int $x;

  public function foo(): int {
    if ($this->x) {
      return $this->x;
    }
    return 0;
  }
}
