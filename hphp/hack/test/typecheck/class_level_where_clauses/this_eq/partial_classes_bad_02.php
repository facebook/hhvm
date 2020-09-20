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
  use PartialC;  // Error! C has to be final
  public function me(): void {}
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
