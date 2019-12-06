<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class C {
  public function __construct(public ?Vector<int> $vec) { }
}

function testit(C $x):void {
  $x->vec = Vector { };
  for ($i = 0; $i < 10; $i++) {
    $x->vec->add($i);
  }
}
