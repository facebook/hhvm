//// modules.php
<?hh


new module A {}
new module B {}
//// A.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.


module A;

class A {
  internal static function f(): void {}
}

function a(): void {
  A::f();
}

//// B.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.


module B;

function b(): void {
  A::f();
}

//// none.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function none(): void {
  A::f();
}
