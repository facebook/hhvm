<?hh // partial
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  public $foo;

  public function test(): void {
    $x = Vector {};
    $this->foo->bar($x);
    $x[0][1];
  }
}
