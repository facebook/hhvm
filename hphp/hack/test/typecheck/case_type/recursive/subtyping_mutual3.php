<?hh
<<file: __EnableUnstableFeatures('case_types', 'recursive_case_types')>>

case type A = B;

case type B = vec<A>;

function expect_A(A $x): void {}
function expect_B(B $x): void {}
function expect_vecA(vec<A> $x): void {}
function expect_vecB(vec<B> $x): void {}

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
  expect_B($a); // nok: A <: B -> nok, need bound on A
  expect_vecA($a); // nok: A <: vec<A> -> nok, need bound on A
  expect_vecB($a); // nok: A <: vec<B> -> nok, need bound on A
  expect_A($b);
  expect_B($b);
  expect_vecA($b); // nok, would need bound on B
  expect_vecB($b); // nok, would need bound on B and/or A
  expect_A($v);
  expect_B($v);
  expect_vecA($v);
  expect_vecB($v);
  expect_A($va);
  expect_B($va);
  expect_vecA($va);
  expect_vecB(
    $va,
  ); // nok: vec<A> <: vec<B> -> vec<A> <: vec<vec<A>> -> A <: vec<A> -> nok, need bound on A
  expect_A(
    $vb,
  ); // ok: vec<B> <: A -> vec<B> <: B -> vec<B> <: vec<A> -> B <: A -> B <: B -> ok
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
  expect_A($vvi); // nok
  expect_B($vvi); // nok
  expect_vecA($vvi); // nok
  expect_vecB($vvi); // nok
}
