<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class A {
  public int $CA;

  public function __construct() {
    $this->CA = 4;
  }
}

interface B {}

function pass((A & B) $a): void {
  $a->CA = 42;
}
