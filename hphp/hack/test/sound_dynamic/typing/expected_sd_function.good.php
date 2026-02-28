<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<__SupportDynamicType>>
class D { }

<<__SupportDynamicType>>
class E {
  public function foo():D { return new D(); }
}

function expectSD(supportdyn<mixed> $n):void { }

function testit():void {
  expectSDTfun(($v) ==> {
    $x = $v[0]->foo();
    expectSD($x);
    return $x;
  });
}

function expectSDTfun(supportdyn<(function(vec<E>):D)> $f): void { }
