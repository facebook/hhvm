<?hh // strict
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

function f1(mixed ...$args): void { }

function f2(int $x, mixed ...$args): void { }

class Foo {
  public function f3(mixed ...$args): void { }

  public function f4(string $x, int $y, bool $z, mixed ...$args): void { }
}
