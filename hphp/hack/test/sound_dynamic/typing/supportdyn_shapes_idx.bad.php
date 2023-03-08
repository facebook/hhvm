<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class C { }

<<__SupportDynamicType>>
class D extends C { }

type TS = supportdyn<shape('a' => int, ?'b' => int, 'c' => C, ?'d' => supportdyn<C>, ...)>;

function expectSDC(supportdyn<C> $x):void { }

function foo(TS $s):void {
  // This shold be rejected because the result of the idx
  // can be C, which is not supportdyn
  expectSDC(Shapes::idx($s, 'd', new C()));
}
