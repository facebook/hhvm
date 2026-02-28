<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class A {
  <<__Const>>
  public int $CA;

  public function __construct() {
    $this->CA = 4;
  }
}

interface B {}

function fail((A & B) $a): void {
  $a->CA = 42;
}
