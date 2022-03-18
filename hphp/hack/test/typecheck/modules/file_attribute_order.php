//// modules.php
<?hh
<<file:__EnableUnstableFeatures('modules')>>

module A {}

//// A1.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.
<<file:__EnableUnstableFeatures('modules'), __Module('A')>>

<<__Internal>>
function f(): void {}


//// A2.php
<?hh

function g(): void {
  f(); // NO ERRORS
}

<<file:__EnableUnstableFeatures('modules'), __Module('A')>>
