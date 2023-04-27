//// modules.php
<?hh


new module X {}
new module Y {}
//// X.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

module X;

internal enum X: int {
  A = 0;
  B = 1;
  C = 2;
}

internal function f1(X $x): void {} // ok

function f2(X $x): void {} // error

internal function f5(): void {
  $x = X::A; // ok
}

function f6(): void {
  $x = X::A; // ok
}


//// Y.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

module Y;

function f3(X $x): void {} // error

function f7(): void {
  $x = X::A; // error
}

//// no-module.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function f4(X $x): void {} // error

function f8(): void {
  $x = X::A; // error
}
