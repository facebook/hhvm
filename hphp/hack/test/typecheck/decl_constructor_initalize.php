//// base.php
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

abstract class Base {
  private bool $b;
  public function __construct() {
    $this->b = false;
  }
}

//// derived.php
<?hh // partial

class Derived extends Base {}
