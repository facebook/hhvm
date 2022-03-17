//// modules.php
<?hh
<<file:__EnableUnstableFeatures('modules')>>

module A {}
//// A.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.
<<file:__EnableUnstableFeatures('modules'), __Module("A")>>

<<__Internal>>
function f(): void {}

//// main.php
<?hh

function main(): void {
  $f = f<>; // error

  $f();
}
