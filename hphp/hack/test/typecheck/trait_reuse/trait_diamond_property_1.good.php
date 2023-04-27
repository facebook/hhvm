<?hh
// (c) Meta Platforms, Inc. and affiliates. All Rigths Reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

// Diamond import of traits defining only non-generic properties is allowed

trait MyTrait1 {
  public int $myprop = 1;
}

trait MyTrait2 {
  use MyTrait1;
}

class MyClass {
  use MyTrait1;
  use MyTrait2;
}
