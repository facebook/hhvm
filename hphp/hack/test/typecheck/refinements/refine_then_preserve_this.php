<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class C {
  public function getit():this {
    return $this;
  }
}

function testit<T>(T $x):T {
  $x as C;
  $y = $x->getit();
  return $y;
}
