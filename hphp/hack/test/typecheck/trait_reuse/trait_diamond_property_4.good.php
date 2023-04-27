<?hh
// (c) Meta Platforms, Inc. and affiliates. All Rigths Reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

// Diamond import of traits defining generic properties at the same type is allowed

trait MyTrait1<T> {
  public ?T $myprop = null;
}

class C {
  use MyTrait1<int>;
}

class D extends C {
  use MyTrait1<int>;
}
