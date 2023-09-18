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

class A {
  internal function __construct() {}
}

function a(): void {
  $a = new A(); // ok
}

//// B.php
<?hh


module B;


function b(): void {
  $a = new A(); // bad
}

//// none.php
<?hh

function none(): void {
  $a = new A(); // bad
}
