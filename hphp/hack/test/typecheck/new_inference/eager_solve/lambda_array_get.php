<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class B { }
class C extends B { }

class Contra<-T> {
  public function __construct(private T $item) { }
}

function applyAndReturn<T1,T2>(T1 $x, (function(T1):T2) $f):(T2,Contra<T1>) {
  $r = ($f)($x);
  return tuple($r,new Contra($x));
}

function expect<T>(T $x):void { }
function expectContraTupleC(Contra<(bool, C)> $c):void { }
function expectContraTupleB(Contra<(bool, B)> $c):void { }

function testtuple(bool $b):void {
  $x1 = applyAndReturn(
    tuple(2, new B()),
    $y ==> { return $y[1]; }
  );
  $x2 = applyAndReturn(
    tuple(2, new B()),
    $y ==> { return $y[1]; }
  );
  $x3 = applyAndReturn(
    tuple(2, new C()),
    $y ==> { return $y[1]; }
  );
  $x4 = applyAndReturn(
    tuple(2, new C()),
    $y ==> { return $y[1]; }
  );
  expectContraTupleB($x1[1]);
  expectContraTupleC($x2[1]);
  expectContraTupleB($x3[1]);
  expectContraTupleC($x4[1]);
}

function testshape(bool $b):void {
  $x1 = applyAndReturn(
    shape('a' => new B()),
    $y ==> { return $y['a']; }
  );
  $x2 = applyAndReturn(
    shape('a' => new B()),
    $y ==> { return $y['a']; }
  );
  $x3 = applyAndReturn(
    shape('a' => new C()),
    $y ==> { return $y['a']; }
  );
  $x4 = applyAndReturn(
    shape('a' => new C()),
    $y ==> { return $y['a']; }
  );
  expect<Contra<shape('a' => B)>>($x1[1]);
  expect<Contra<shape('a' => C)>>($x2[1]);
  expect<Contra<shape('a' => B)>>($x3[1]);
  expect<Contra<shape('a' => C)>>($x4[1]);
}

function testshapeopt(bool $b):void {
  $x1 = applyAndReturn(
    shape('a' => new B()),
    $y ==> { return $y['a'] ?? new C(); }
  );
  $x2 = applyAndReturn(
    shape('a' => new B()),
    $y ==> { return $y['a'] ?? new C(); }
  );
  $x3 = applyAndReturn(
    shape('a' => new C()),
    $y ==> { return $y['a'] ?? new C(); }
  );
  $x4 = applyAndReturn(
    shape('a' => new C()),
    $y ==> { return $y['a'] ?? new C(); }
  );
  expect<Contra<shape(?'a' => B)>>($x1[1]);
  expect<Contra<shape(?'a' => C)>>($x2[1]);
  expect<Contra<shape(?'a' => B)>>($x3[1]);
  expect<Contra<shape(?'a' => C)>>($x4[1]);
}

function testshapeopt2(shape(?'a' => B) $s):void {
  $x1 = applyAndReturn(
    $s,
    $y ==> { return $y['a'] ?? new C(); }
  );
  $x2 = applyAndReturn(
    $s,
    $y ==> { return $y['a'] ?? new C(); }
  );
  $x3 = applyAndReturn(
    $s,
    $y ==> { return $y['a'] ?? new C(); }
  );
  $x4 = applyAndReturn(
    $s,
    $y ==> { return $y['a'] ?? new C(); }
  );
  expect<Contra<shape(?'a' => B)>>($x1[1]);
  expect<Contra<shape(?'a' => C)>>($x2[1]);
  expect<Contra<shape(?'a' => B)>>($x3[1]);
  expect<Contra<shape(?'a' => C)>>($x4[1]);
}

type ST = shape(?'a' => B, 'b' => C);

function testshapeopt3(shape(?'a' => B, 'b' => C) $s, Contra<shape(?'a' => B, 'b' => C)> $s2):void {
  $x1 = applyAndReturn(
    $s,
    $y ==> { invariant(Shapes::idx($y, 'a') !== null, 'error'); return tuple($y['a'], $y['b']); }
  );
  $x2 = applyAndReturn(
    $s,
    $y ==> { return $y['a'] ?? new C(); }
  );
  $x3 = applyAndReturn(
    $s,
    $y ==> { return $y['a'] ?? new C(); }
  );
  $x4 = applyAndReturn(
    $s,
    $y ==> { return $y['a'] ?? new C(); }
  );
  expect<Contra<shape(?'a' => B, 'b' => C)>>($x1[1]);
  expect<Contra<shape(?'a' => C)>>($x2[1]);
  expect<Contra<shape(?'a' => B)>>($x3[1]);
  expect<Contra<shape(?'a' => C)>>($x4[1]);
}

function testvec(ConstVector<string> $cv):void {
  $x1 = applyAndReturn(
    vec["a"],
    $y ==> { return $y[1]; }
  );
  expect<Contra<AnyArray<int,string>>>($x1[1]);
  $x2 = applyAndReturn(
    $cv,
    $y ==> { return $y[1]; }
  );
  expect<Contra<ConstVector<string>>>($x2[1]);
}

function teststring():void {
  $x8 = applyAndReturn(
    "abc",
    $y ==> { return $y[3]; }
  );
  expect<Contra<string>>($x8[1]);
}
