//// modules.php
<?hh
<<file:__EnableUnstableFeatures('modules')>>

new module A {}
new module B {}
//// A.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.
<<file:__EnableUnstableFeatures('modules')>>
module A;

internal interface A {}

interface A2 {}

interface A3 {
  internal function f(): void;
  // Ok! But it's not possible to implement this outside the module
}

//// B.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.
<<file:__EnableUnstableFeatures('modules')>>
module B;

class B implements A {} // Bad

class B2 implements A2 {} // Ok
