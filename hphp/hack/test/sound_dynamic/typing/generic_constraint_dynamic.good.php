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

function testit():void {
  $x = new C<int>(3);
  $y = make<string>("A");
  //$a = new C(3);
  //$b = make("A");
}
