<?hh
<<file: __EnableUnstableFeatures('case_types', 'recursive_case_types')>>

case type A as vec<A> = vec<A>;

function expect_A(A $x): void {}
function expect_vecA(vec<A> $x): void {}
function expect_vecvecA(vec<vec<A>> $x): void {}

function foo(
  A $a,
  vec<nothing> $v,
  vec<A> $va,
  vec<vec<nothing>> $vv,
  vec<vec<A>> $vva,
): void {
  expect_A($a);
  expect_A($v);
  expect_A($va);
  expect_A($vv);
  expect_A($vva);
  expect_vecA($a); // ok: A <: vec<A> -> vec<A> <: vec<A> -> ok
  expect_vecA($v);
  expect_vecA($va);
  expect_vecA($vv);
  expect_vecA($vva);
  expect_vecvecA(
    $a,
  ); // ok: A <: vec<vec<A>> -> vec<A> <: vec<vec<A>> -> A <: vec<A> -> vec<A> <: vec<A> -> ok
  expect_vecvecA(
    $v,
  ); // ok: vec<nothing> <: vec<vec<A>> -> nothing <: vec<A> -> ok
  expect_vecvecA(
    $va,
  ); // ok: vec<A> <: vec<vec<A>> -> A <: vec<A> -> vec<A> <: vec<A> -> ok
  expect_vecvecA(
    $vv,
  ); // ok: vec<vec<nothing>> <: vec<vec<A>> -> vec<nothing> <: vec<A> -> nothing <: A -> ok
  expect_vecvecA($vva); // ok: vec<vec<A>> <: vec<vec<A>> -> ... -> ok
}
