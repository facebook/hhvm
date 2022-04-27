//// modules.php
<?hh
<<file:__EnableUnstableFeatures('modules')>>

new module A {}
//// A.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.
<<file:__EnableUnstableFeatures('modules')>>
module A;

internal function f(): void {}

//// main.php
<?hh

function main(): void {
  $f = f<>; // error

  $f();
}
