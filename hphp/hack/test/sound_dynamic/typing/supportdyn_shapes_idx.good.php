<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class C { }

type TS = supportdyn<shape('a' => int, ?'b' => int, 'c' => C, ...)>;

function expectSDNC(?supportdyn<C> $x):void { }

function expectSDC(supportdyn<C> $x):void { }

function expectSDM(supportdyn<mixed> $x):void { }

function foo(TS $s):void {
  expectSDNC(Shapes::idx($s, 'c'));
  expectSDM(Shapes::idx($s, 'd'));
  expectSDC(Shapes::idx($s, 'c', new C()));
  expectSDM(Shapes::idx($s, 'd', 5));
}
