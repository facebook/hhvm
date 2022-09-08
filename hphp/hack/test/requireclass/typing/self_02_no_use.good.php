<?hh
// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

<<file:__EnableUnstableFeatures('require_class')>>

trait T {
  require class C;

  public static function foo(): void { self::bar(); }
}

class D {
  public static function bar(): void { }
}

final class C extends D {
}
