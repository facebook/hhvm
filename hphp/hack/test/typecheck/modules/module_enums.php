//// X.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.
<<file:__EnableUnstableFeatures('modules'), __Module('X')>>

<<__Internal>>
enum X: int {
  A = 0;
  B = 1;
  C = 2;
}

<<__Internal>>
function f1(X $x): void {} // ok

function f2(X $x): void {} // error

<<__Internal>>
function f5(): void {
  $x = X::A; // ok
}

function f6(): void {
  $x = X::A; // ok
}


//// Y.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.
<<file:__EnableUnstableFeatures('modules'), __Module('Y')>>

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
