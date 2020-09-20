<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

interface I { }
interface J { }
interface IGet<T> {
  public function get():T;
}
class C<T as I> implements IGet<T> {
  public function __construct(private T $item) { }
  public function get():T { return $this->item; }
}
class D<T as J> implements IGet<T> {
  public function __construct(private T $item) { }
  public function get():T { return $this->item; }
}

class E implements I { }
function expectJ(J $x):void { }
function testit(mixed $m, bool $b):void {
  if ($b) {
    invariant($m is C<_>, "C");
  } else {
    invariant($m is D<_>, "D");
  }
  $a = $m->get();
  expectJ($a);
}

<<__EntryPoint>>
function breakit():void {
  testit(new C(new E()), true);
}
