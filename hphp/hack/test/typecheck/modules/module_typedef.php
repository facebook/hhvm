//// modules.php
<?hh
<<file:__EnableUnstableFeatures('modules')>>

module A {}
module B {}
//// A.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<file:__EnableUnstableFeatures('modules'), __Module('A')>>

<<__Internal>>
type Ty = int;

<<__Internal>>
newtype TyNew = int;

// In Signatures

class A {
  public function a(Ty $x): void {} // error

  <<__Internal>>
  public function b(Ty $x): void {} // ok

  public function c(TyNew $x): void {} // error

  <<__Internal>>
  public function d(TyNew $x): void {} // ok
}

// Visibility

function f(): void {
  $x = 1 as Ty; // ok
}

function g(): void {
  $x = 1 as TyNew; // ok
}


//// B.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<file:__EnableUnstableFeatures('modules'), __Module('B')>>

function h(Ty $x): void {} // error

function h_new(TyNew $x): void {} // error

//// no-module.php
<?hh

function j(Ty $x): void {} // error

function j_new(TyNew $x): void {} // error
