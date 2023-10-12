<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class Inv<T> {
  public function __construct(public T $value) {}
}

function test(shape('x' => int) $shape): string {
  $shape = (new Inv($shape))->value;
  return (string)$shape['x'];
}
