//// modules.php
<?hh
<<file:__EnableUnstableFeatures('modules')>>

new module A {}
//// A.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.
<<file:__EnableUnstableFeatures('modules'), __Module('A')>>

class A {
  internal function foobar(): void {}
}

//// f.php
<?hh

class B extends A {
  <<__Override>>
  public function foobar(): void {
    parent::foobar();
  }
}
