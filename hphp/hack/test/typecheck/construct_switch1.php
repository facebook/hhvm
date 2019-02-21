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
  private int $x;

  public function __construct() {
    switch (1) {
      case 1:
        $this->x = 1;
        break;
      default:
        invariant_violation('fail');
        break;
    }
  }
}
