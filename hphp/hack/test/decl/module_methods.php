//// modules.php
<?hh
<<file:__EnableUnstableFeatures('modules')>>

module A {}

//// A.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<file:__EnableUnstableFeatures('modules'), __Module("A")>>

class A {
  <<__Internal>>
  public function f(): void {}
}

//// main.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function main(): void {
  $a = new A();
  $a->f();
}
