<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<__SupportDynamicType>>
class C<<<__RequireDynamic>>T as dynamic> {
  public function __construct(private T $item) { }
  public function get():T {
    return $this->item;
  }
}

function make<<<__RequireDynamic>>T as dynamic>(T $x): C<T> {
  return new C<T>($x);
}

class D { }

function testit():void {
  $x = new C<D>(new D());
  $y = make<D>(new D());
  $a = new C(new D());
  $b = make(new D());
}
