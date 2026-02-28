<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class A {
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
  public function pass((T | C) $a): void {
    $a->CA = 42;
  }
}
