//// modules.php
<?hh
<<file:__EnableUnstableFeatures('modules')>>

module A {}
module B {}
//// A.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.
<<file:__EnableUnstableFeatures('modules'), __Module('A')>>

class A {
  public function pub(): void {}

  <<__Internal>>
  public function internal(): void {}
}

class A2 extends A {
  // This is allowed because it's the same module
  <<__Override, __Internal>>
  public function internal(): void {}
}

// This is illegal, overriding a public method to be internal

class A3 extends A {
  <<__Override, __Internal>>
  public function pub(): void {}
}

//// B.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.
<<file:__EnableUnstableFeatures('modules'), __Module('B')>>

class B extends A {
  <<__Override, __Internal>>
  public function pub(): void {}
}


// This is illegal because it is overriding an internal method from a different
// module
class B2 extends A {
  <<__Override, __Internal>>
  public function internal(): void {}
}
