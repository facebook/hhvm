<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.
<<file:__EnableUnstableFeatures('modules')>>

<<__Module("A")>>
class A {
  <<__Internal>>
  public function f(): void {}

  public function g(): void { $this->f(); /* ok */ }

}

<<__Module("B")>>
class B {
  public function f(A $a): void {
    $a->f(); // error
  }
}

<<__Module("A")>>
function a(A $a): void { $a->f(); /* ok */ }

<<__Module("B")>>
function b(A $a): void { $a->f(); /* error */ }

function main(A $a): void { $a->f(); /* error */ }
