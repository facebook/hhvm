//// partial.php
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
  public function any() {}
}

//// strict.php
<?hh // strict

function f(): void {
  $c = new C();
  $c->any()->foo();
}
