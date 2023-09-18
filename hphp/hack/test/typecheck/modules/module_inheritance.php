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
  public function pub(): void {}

  internal function internal(): void {}
}

class A2 extends A {
  // This is allowed because it's the same module
  <<__Override>>
  internal function internal(): void {}
}

// This is illegal, overriding a public method to be internal

class A3 extends A {
  <<__Override>>
  internal function pub(): void {}
}

//// B.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

module B;

class B extends A {
  <<__Override>>
  internal function pub(): void {}
}


// This is illegal because it is overriding an internal method from a different
// module
class B2 extends A {
  <<__Override>>
  internal function internal(): void {}
}
