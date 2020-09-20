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
}

final class C2 {
  use PartialC;  // Error! Must be class C
}

interface I {
  public static function get(): this;
}

trait PartialC implements I where this = C {
  public static function get(): C {
    return new C();
  }
  public function foo(): void { $this->me(); }
}
