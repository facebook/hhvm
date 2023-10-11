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

trait T1 {
  protected int $i;
}

trait T2 {
  use T1;
  protected int $j;
}

class B {
  use T2;
  public function __construct() {
    $this->i = 1;
    $this->j = 2;
  }
}

class C extends B {
  public function test(): void {
    $this->i = 2;
  }
}
