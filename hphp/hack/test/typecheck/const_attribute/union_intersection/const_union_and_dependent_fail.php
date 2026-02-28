<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class A {
  <<__Const>>
  public int $CA;

  public function __construct() {
    $this->CA = 4;
  }
}

class B {
  public int $CA;

  public function __construct() {
    $this->CA = 0;
  }
}

abstract class R {
  abstract const type T as A;
  abstract public function get() : (this::T | B);
}

function pass(R $r): void {
  $a = $r->get();
  $a->CA = 42;
}
