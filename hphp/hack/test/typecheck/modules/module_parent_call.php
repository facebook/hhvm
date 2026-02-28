//// modules.php
<?hh


new module A {}
//// A.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

module A;

class A {
  internal function foobar(): void {}
  protected internal function baz(): void {}
}

//// f.php
<?hh

class B extends A {
  <<__Override>>
  public function foobar(): void {
    parent::foobar();
  }

  <<__Override>>
  public function baz(): void {
    parent::baz();
  }
}
