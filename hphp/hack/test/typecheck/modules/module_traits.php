<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.
<<file:__EnableUnstableFeatures('modules')>>

<<__Module("A"), __Internal>>
trait T1 {}

<<__Module("A")>>
trait T2 {
  <<__Internal>>
  public int $x = 0;
}

// Using an internal trait ///////////////////////

<<__Module("A")>>
class A {
  use T1; // ok
}

<<__Module("B")>>
class B {
  use T1; // error
}

class C {
  use T1; // error
}

// Leaking internal types in public trait ////////

<<__Module("A"), __Internal>>
class D {}

<<__Module("A")>>
trait T3 {
  public function f(D $d): void {} // can't use D here
}
