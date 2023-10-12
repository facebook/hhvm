<?hh //strict
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class A {
  <<__Const>>
  public int $CA;

  public function __construct() {
    $this->CA = 4;
  }
}

class Client {
  public ~A $a;

  public function __construct() {
    $this->a = new A();
  }

  public function fail(): void {
    $this->a->CA = 42;
  }
}
