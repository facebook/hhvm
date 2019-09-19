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

trait Tu {
  private function me(): void {}
  private function accessible(): void {}
}

class C { use Tu; }

trait Tv where this as C {
  private function me(): void {}
  protected function accessible(): void {}
}

class D extends C { use Tv; }

trait Tw where this as D {
  public function foo(): void {
    $this->accessible();
    $this->me();
  }
}

trait Tw2 {
  require extends D;

  public function foo(): void {
    $this->accessible();
    $this->me();    // Same error as in T.
  }
}
