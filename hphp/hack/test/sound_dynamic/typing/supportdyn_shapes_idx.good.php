<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class C { }

<<__SupportDynamicType>>
class D extends C { }

type TS = supportdyn<shape('a' => int, ?'b' => int, 'c' => C, ?'d' => supportdyn<C>, ...)>;

function expectSDNC(?supportdyn<C> $x):void { }

function expectSDC(supportdyn<C> $x):void { }

function expectSDM(supportdyn<mixed> $x):void { }

function expectShape(shape('c' => supportdyn<C>, ...) $_):void { }
function foo(TS $s):void {
  expectSDNC(Shapes::idx($s, 'c'));
  expectSDM(Shapes::idx($s, 'd'));
  // This is legal, because if `d` is present,
  // then it's supportdyn<C>. If it's absent,
  // then it's D which is SDT.
  expectSDC(Shapes::idx($s, 'd', new D()));
  expectSDM(Shapes::idx($s, 'e', 5));
}
