//// modules.php
<?hh


new module A {}

//// A.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.


module A;

class A {
  internal function f(): void {}
}

//// main.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function main(): void {
  $a = new A();
  $a->f();
}
