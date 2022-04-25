//// modules.php
<?hh
<<file:__EnableUnstableFeatures('modules')>>

new module A {}
new module B {}
//// A.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<file:__EnableUnstableFeatures('modules'), __Module('A')>>

class A {
  internal function __construct() {}
}

function a(): void {
  $a = new A(); // ok
}

//// B.php
<?hh

<<file:__EnableUnstableFeatures('modules'), __Module('B')>>

<<__Module("B")>>
function b(): void {
  $a = new A(); // bad
}

//// none.php
<?hh

function none(): void {
  $a = new A(); // bad
}
