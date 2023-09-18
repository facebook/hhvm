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
  internal static int $x = 0;
}

function a(): void {
  A::$x = 1;
}


//// B.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.


module B;

function b(): void {
  A::$x = 1;
}

//// no-module.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function none(): void {
  A::$x = 1;
}
