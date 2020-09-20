<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class Ref<T> {
  public function __construct(public T $value) {}
}

function test(shape('a' => int, 'b' => int, ...) $s): void {
  Shapes::removeKey(inout $s, 'a');
  $r = new Ref($s);
  $r->value['b'] = 0;
  $r->value = shape('a' => 1, 'b' => 2);
}
