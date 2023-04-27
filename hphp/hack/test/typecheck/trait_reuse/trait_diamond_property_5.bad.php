<?hh
// (c) Meta Platforms, Inc. and affiliates. All Rigths Reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

// Multiple import of traits defining generic properties at different types is forbidden

trait MyTrait1<T> {
  public ?T $myprop = null;
}

class C {
  use MyTrait1<int>;
}

class D extends C {
  use MyTrait1<string>;
}
