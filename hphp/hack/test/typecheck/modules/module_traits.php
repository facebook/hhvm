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

internal trait T1 {}

trait T2 {
  // Error: cannot have internal traits
  internal int $x = 0;
  // Error: cannot have internal traits
  internal function lol(): void {}
}

// Using an internal trait

class A {
  use T1; // ok
}
// Leaking internal trait

internal class D {}

trait T3 {
  public function f(D $d): void {} // can't use D here
}

//// B.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

module B;

class B {
  use T1; // error
}

//// C.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class C {
  use T1; // error
}

//// E.php
<?hh

class E {
  use T2;
  public function checkMembers(): void {
    $this->x; // ERROR: `$this->x` is internal to `A`
    $this->lol(); // ERROR: `$this->lol` is internal to `A`
  }
}
