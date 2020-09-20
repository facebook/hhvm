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

abstract class C {
  abstract public function foo(mixed $x): num;
}

trait T where this as C {
  abstract public function foo(num $x): mixed;
  public function call_foo(mixed $x): num {
    return $this->foo($x);
  }
}

abstract class D extends C {
  use T;
  abstract public function foo(mixed $x): int;
  public function call_foo(mixed $x): int {
    return $this->foo($x);
  }
}
