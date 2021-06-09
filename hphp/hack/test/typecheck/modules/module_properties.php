<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<file:__EnableUnstableFeatures('modules')>>

<<__Module("A")>>
class A {
  <<__Internal>>
  public int $x = 0;
}

function f(A $a): void {
  $a->x = 123;
}
