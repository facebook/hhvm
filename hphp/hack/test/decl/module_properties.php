//// modules.php
<?hh
<<file:__EnableUnstableFeatures('modules')>>

module A {}

//// A.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<file:__EnableUnstableFeatures('modules'), __Module("A")>>

class A {
  <<__Internal>>
  public int $x = 0;
}

//// f.php
<?hh

function f(A $a): void {
  $a->x = 123;
}
