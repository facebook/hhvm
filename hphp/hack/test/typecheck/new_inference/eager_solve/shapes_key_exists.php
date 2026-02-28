<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Inv<T> {
  public function __construct(public T $value) {}
}

function test(shape(?'x' => int, ...) $s): int {
  $s = (new Inv($s))->value;
  return Shapes::keyExists($s, 'x') ? $s['x'] : 0;
}
