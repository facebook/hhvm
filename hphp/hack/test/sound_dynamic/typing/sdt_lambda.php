<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<__SupportDynamicType>>
class D { }

<<__SupportDynamicType>>
class C {
  public function foo():D { return new D(); }
}

function expectD(D $n):void { }

function testit():void {
  $f = <<__SupportDynamicType>> (vec<C> $c):D ==> {
    $x = $c[0]->foo();
    // should be an error in check under dynamic
    expectD($x);
    return $x;
  };
}
