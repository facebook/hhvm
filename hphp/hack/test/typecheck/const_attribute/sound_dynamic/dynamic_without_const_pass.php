<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class A {
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

  public function pass(): void {
    $this->a->CA = 42;
  }
}
