<?hh
// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

<<file:
  __EnableUnstableFeatures('method_trait_diamond', 'require_class'),
>>

trait T1 {
  public function foo(): void {}
}

trait T2 {
  use T1;
  require class C;
}

final class C {
  use T2;
  public function foo(): void {}
}
