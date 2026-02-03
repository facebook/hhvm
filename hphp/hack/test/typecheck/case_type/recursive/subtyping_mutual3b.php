<?hh
<<file: __EnableUnstableFeatures('case_types', 'recursive_case_types')>>

case type A as B = B;

case type B as A = vec<A>; // constraint ok: vec<A> <: A -> vec<A> <: B -> vec<A> <: vec<A> -> A <: A -> ok

function expect_A(A $x): void {}
function expect_B(B $x): void {}
function expect_vecA(vec<A> $x): void {}
function expect_vecB(vec<B> $x): void {}
function expect_vecvecA(vec<vec<A>> $x): void {}
function expect_vecvecB(vec<vec<B>> $x): void {}

function foo(
  A $a,
  B $b,
  vec<nothing> $v,
  vec<A> $va,
  vec<B> $vb,
  vec<vec<nothing>> $vv,
  vec<vec<A>> $vva,
  vec<vec<B>> $vvb,
  vec<vec<int>> $vvi,
): void {
  expect_A($a);
  expect_B($a);
  expect_vecA(
    $a,
  ); // nok: A <: vec<A> -> B <: vec<A> -> A <: vec<A> -> recursing -> nok
  expect_vecB($a);
  expect_vecvecA(
    $a,
  ); // nok: A <: vec<vec<A>> -> B <: vec<vec<A>> -> A <: vec<vec<A>> -> recursing -> nok
  expect_A($b);
  expect_B($b);
  expect_vecA($b);
  expect_vecB($b);
  expect_A($v);
  expect_B($v);
  expect_vecA($v);
  expect_vecB($v);
  expect_A($va);
  expect_B($va);
  expect_vecA($va);
  expect_vecB($va);
  expect_A($vb);
  expect_B($vb);
  expect_vecA($vb);
  expect_vecB($vb);
  expect_A($vv);
  expect_B($vv);
  expect_vecA($vv);
  expect_vecB($vv);
  expect_A($vva);
  expect_B($vva);
  expect_vecA($vva);
  expect_vecB($vva);
  expect_A($vvb);
  expect_B($vvb);
  expect_vecA($vvb);
  expect_vecB($vvb);

  // these are wrong
  expect_A($vvi);
  expect_B($vvi);
  expect_vecA($vvi);
  expect_vecB($vvi);
}
