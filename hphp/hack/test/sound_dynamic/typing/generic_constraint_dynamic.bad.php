<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<__SoundDynamicCallable>>
class C<T as dynamic> {
  public function __construct(private T $item) { }
  public function get():T {
    return $this->item;
  }
}

function make<T as dynamic>(T $x): C<T> {
  return new C<T>($x);
}

class D { }

function testit():void {
  $x = new C<D>(new D());
  $y = make<D>(new D());
  $a = new C(new D());
  $b = make(new D());
}
