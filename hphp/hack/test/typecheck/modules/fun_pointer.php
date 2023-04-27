//// modules.php
<?hh


new module A {}
//// A.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

module A;

internal function f(): void {}

//// main.php
<?hh

function main(): void {
  $f = f<>; // error

  $f();
}
