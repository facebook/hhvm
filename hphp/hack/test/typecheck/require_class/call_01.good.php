<?hh
// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

<<file:__EnableUnstableFeatures('require_class')>>

trait T {
  require class C;

  public function foo(): void { $this->bar(); }
}

final class C {
  use T;

  public function bar(): void { }
}
