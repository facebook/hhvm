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

internal type Ty = int;

internal newtype TyNew = int;

// In Signatures

class A {
  public function a(Ty $x): void {} // error

  internal function b(Ty $x): void {} // ok

  public function c(TyNew $x): void {} // error

  internal function d(TyNew $x): void {} // ok
}

// Visibility

function f(): void {
  $x = 1 as Ty; // ok
}

function g(): void {
  $x = 1 as TyNew; // ok
}

internal function k(Ty $x, TyNew $y): void {
  $z = $x + 5; // ok
  $z = $y + 6; // ok
}


//// B.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.


module B;

function h(Ty $x): void {} // error

function h_new(TyNew $x): void {} // error

//// no-module.php
<?hh

function j(Ty $x): void {} // error

function j_new(TyNew $x): void {} // error

//// still-A.php
<?hh

module A;
internal function k2(Ty $x, TyNew $y): void {
  $z = $x + 5; // ok
  $z = $y + 6; // error, opaque outside of file level type alias
}
