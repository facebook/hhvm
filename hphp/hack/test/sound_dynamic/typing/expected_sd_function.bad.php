<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<__SupportDynamicType>>
class D { }

<<__SupportDynamicType>>
class E {
  public function foo():D { return new D(); }
}

function expectD(D $n):void { }

function testit():void {
  expectSDTfun(($v) ==> {
    $x = $v[0]->foo();
    // should be an error in check under dynamic
    expectD($x);
    return $x;
  });
}

function expectSDTfun(supportdyn<(function(vec<E>):D)> $f): void { }
