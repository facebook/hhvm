<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {}
class B {}
interface C {}
interface D {}
class E implements C, D {}

function test_splat_args(): void {
  $empty = vec[];
  $a = vec[new A(), new A()];
  $b = vec[new B(), new B()];
  $e = vec[new E(), new E()];
  $v = vec[new B(), new E()];

  $aa = tuple(new A(), new A());
  $ab = tuple(new A(), new B());
  $bb = tuple(new B(), new B());
  $be = tuple(new B(), new E());
  $ee = tuple(new E(), new E());

  // OK
  expect_As(...$a);
  expect_AB(...$ab);
  expect_ABs(new A(), ...$b);
  expect_ABs(new A(), new B(), ...$b);
  expect_ABE(new A(), ...$be);
  expect_ABBBBs(new A(), new B(), new B(), ...$bb);

  // Error
  expect_AB(...$aa);
  expect_ABs(new A(), ...$v);
  expect_ABs(new A(), new B(), ...$v);
  expect_ABE(new A(), ...$v);
  expect_ACD(new A(), ...$be);
}

function f(Traversable<E> $e): void {
  // OK
  expect_ACDs(new A(), new E(), ...$e);

  // Error
  expect_ABs(new A(), ...$e);
}

function expect_As(A ...$a): void {}
function expect_AB(A $a, B $b): void {}
function expect_ABs(A $a, B ...$b): void {}
function expect_ABBBBs(A $a, B $b1, B $b2, B $b3, B ...$b): void {}
function expect_ABE(A $a, B $b, E $e): void {}
function expect_ACD(A $a, C $c, D $d): void {}
function expect_ACDs(A $a, C $c, D ...$d): void {}
