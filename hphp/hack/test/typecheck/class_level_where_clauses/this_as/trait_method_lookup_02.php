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
  static public int $x = 0;
  abstract static public function foo(): num;
}

trait T where this as C {
  static public function foo(): mixed {
    return static::$x;
  }
  static public function call_foo(): num {
    return static::foo();  // mixed & num = num
  }
  static public function use_smember_from_C(): int {
    return static::$x;  // num & int = int
  }
}

abstract class D extends C {
  use T;
// can method foo defined in trait T be more general than that in class C?
// For interface, it is ok. But for trait?
}
