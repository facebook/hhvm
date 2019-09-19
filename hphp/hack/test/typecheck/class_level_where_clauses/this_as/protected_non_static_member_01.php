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
  private function me(): void {}
  protected function accessible(): void {}
}

trait T where this as C {
  public function foo(): void {
    $this->accessible();
    $this->me();  // Error!
  }
}

trait T2 {
  require extends C;

  public function foo(): void {
    $this->accessible();
    $this->me();  // Error!
  }
}
