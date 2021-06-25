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
  private function accessible(): void {}
}

class D {
  private function me(): void {}
}

trait Tu where this as D {}

class E extends D {
  use Tu;
  protected function accessible(): void {}
}

trait T where this as E {
  public function foo(): void {
    $this->accessible();
    $this->me(); // Error!
  }
}

trait T2 where this as C {
  public function foo(): void {
    $this->accessible(); // Error
  }
}

trait Treq {
  require extends C;
  require extends E;

  public function foo(): void {
    $this->accessible();
    $this->me();  // Same error as in T.
  }
}
