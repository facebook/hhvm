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

class C {
  private ?int $x;

  public function foo(): int {
    switch (1) {
      case 1:
        $this->x = 1;
        break;
      default:
        $this->x = 2;
        break;
    }

    return $this->x;
  }
}
