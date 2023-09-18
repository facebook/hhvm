//// module_A.php
<?hh
new module A {}

//// module_B.php
<?hh
new module B {}

//// A.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

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

module B;

class B implements A {} // Bad

class B2 implements A2 {} // Ok
