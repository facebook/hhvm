<?hh
// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

<<file: __EnableUnstableFeatures('require_class')>>

interface IPajoux1 {
  public function foo(int $a): void;
}

interface IPajoux2 {
  public function foo(int $a, ?int $b = null): void;
}

final class Pajoux {
  use TPajoux;
  public function foo(int $a, ?int $b = null): void {}
}

trait TPajoux implements IPajoux1, IPajoux2 {
  public function foo(int $a): void {}
  require class Pajoux;
}
