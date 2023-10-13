<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Inv<T> {
  public function __construct(public T $value) {}
}

function test_Vector(): void {
  $v = (new Inv(Vector {1, 2}))->value;
  list($_, $_) = $v;
}

function test_vec(): void {
  $v = (new Inv(vec[1, 2]))->value;
  list($_, $_) = $v;
}

function test_varray(): void {
  $v = (new Inv(varray[1, 2]))->value;
  list($_, $_) = $v;
}

function test_Pair(): void {
  $p = (new Inv(Pair {1, 'two'}))->value;
  list($_, $_) = $p;
}
