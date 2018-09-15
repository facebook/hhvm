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

class Foo {
  protected static function __callStatic(string $name, array $args): int {
    return 123;
  }
}

function bar(): int {
  return Foo::batman();
}
