<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class A {
  <<__Const>>
  public int $CA;

  public function __construct() {
    $this->CA = 4;
  }
}

newtype B = A;

class C {
  public int $CA;

  public function __construct() {
    $this->CA = 0;
  }
}

function fail((B | C) $a): void {
  $a->CA = 42;
}
