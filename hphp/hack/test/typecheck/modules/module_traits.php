//// A.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.
<<file:__EnableUnstableFeatures('modules'), __Module('A')>>

<<__Internal>>
trait T1 {}

trait T2 {
  <<__Internal>>
  public int $x = 0;
}

// Using an internal trait

class A {
  use T1; // ok
}
// Leaking internal types in public trait

<<__Internal>>
class D {}

trait T3 {
  public function f(D $d): void {} // can't use D here
}

//// B.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.
<<file:__EnableUnstableFeatures('modules'), __Module('B')>>

class B {
  use T1; // error
}

//// C.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class C {
  use T1; // error
}
