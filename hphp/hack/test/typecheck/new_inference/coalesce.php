<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class Inv<T> {
  public function __construct(public T $value) {}
}

function test(bool $b): void {
  $x = new Inv(42);
  $v = $b ? $x->value : null;
  $x->value = $v ?? 'foo';
}
