<?hh
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

class Base {
  protected function __call(string $name, array $args): int {
    return 123;
  }

  public static function __callStatic(string $name, array $args): string {
    return "hello";
  }
}

class Child extends Base {
  public function foo(): int {
    return $this->getFoo();
  }

  public static function bar(): string {
    return Child::getFoo();
  }
}
