//// modules.php
<?hh
<<file:__EnableUnstableFeatures('modules')>>

new module A {}
new module B {}

//// A1.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.
<<file:__EnableUnstableFeatures('modules'), __Module('A'), __Module('B')>>

internal function f(): void {}


//// A2.php
<?hh
<<file:__EnableUnstableFeatures('modules'), __Module('A')>>

function g(): void { f(); }
