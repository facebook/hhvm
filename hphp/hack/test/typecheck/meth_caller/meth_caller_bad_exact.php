<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class Base {
  public function get(): this { return $this; }
  public function set(this $_x): void {}
}
class Child extends Base {}

function use_meth_caller(Base $base): void {
  $f = meth_caller(Base::class, 'get');
  $exact_base = new Base();
  //$base->set($exact_base); // Correctly reports error
  $nonexact_base = $f($base);
  $nonexact_base->set($exact_base); // No error, fatals at runtime
}

<<__EntryPoint>>
function breakit(): void {
  $child = new Child();
  use_meth_caller($child);
}
