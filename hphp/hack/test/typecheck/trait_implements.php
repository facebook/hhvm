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

interface I {
  public function bar(): void;
}

trait X implements I {
  public function foo(): void {
    $this->bar();
  }
}

class Y {
  use X;
}
