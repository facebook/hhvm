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

class C {
  private string $x;

  public function __construct() {
    // use of $this->x before it is initialized
    // through a function call
    $this->init();
  }

  private function init(): void {
    $this->x = $this->x;
  }
}
