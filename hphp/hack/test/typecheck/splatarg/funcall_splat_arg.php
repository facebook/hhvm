<?hh // strict
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

  expect_A(...$empty);
  expect_A(...$a);
  expect_As(...$a);
  expect_AB(new A(), ...$b);
  expect_ABBB(new A(), ...$b);
  expect_ABBB(new A(), new B(), ...$b);
  expect_ABs(new A(), ...$b);
  expect_ABs(new A(), new B(), ...$b);
  expect_ABBBBs(new A(), new B(), ...$b);
  expect_ACD(new A(), ...$e);
  expect_ACDs(new A(), ...$e);

  // failing tests
  expect_AB(new A(), new B(), ...$b);
  expect_ABE(new A(), ...$v);
}

function f(Traversable<E> $e): void {
  expect_ACD(new A(), ...$e);
}

function expect_A(A $a): void {}
function expect_As(A ...$a): void {}
function expect_AB(A $a, B $b): void {}
function expect_ABBB(A $a, B $b1, B $b2, B $b3): void {}
function expect_ABs(A $a, B ...$b): void {}
function expect_ABBBBs(A $a, B $b1, B $b2, B $b3, B ...$b): void {}
function expect_ABE(A $a, B $b, E $e): void {}
function expect_ACD(A $a, C $c, D $d): void {}
function expect_ACDs(A $a, C $c, D ...$d): void {}
