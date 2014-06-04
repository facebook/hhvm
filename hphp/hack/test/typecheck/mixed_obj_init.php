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

class MixedInitTest {
  protected mixed $foo;
  public function __construct() {
    // make sure we are allowed to call functions
    $this->something();
  }
  protected function something(): void {
  }
}
