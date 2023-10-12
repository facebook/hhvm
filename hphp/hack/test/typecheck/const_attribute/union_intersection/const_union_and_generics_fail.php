<?hh //strict
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class A {
  <<__Const>>
  public int $CA;

  public function __construct() {
    $this->CA = 4;
  }
}

class C {
  public int $CA;

  public function __construct() {
    $this->CA = 0;
  }
}

class Client<T as A, Tx as Ty, Ty as Tx> {
  // the mutual dependency between Tx and Ty will test type-checker termination
  public function fail((T | C) $a): void {
    $a->CA = 42;
  }
}
