<?hh
// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

<<file:__EnableUnstableFeatures('require_class')>>

trait T {
  require class C;

  private int $i;

  public function bar(): void {
    $this->i;
  }
}

final class C {
  use T;

  public function __construct(private int $i) {}
}
