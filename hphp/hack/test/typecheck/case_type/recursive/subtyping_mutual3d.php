<?hh
<<file: __EnableUnstableFeatures('case_types', 'recursive_case_types')>>

case type A as B = B;

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
  expect_A($a); // ok: A <: A
  expect_B($a); // ok: A <: B -> ok with bound on A
  expect_vecA($a); // nok: A <: vec<A> -> B <: vec<A> -> nok, need bound on B
  expect_vecB($a); // nok: A <: vec<B> -> B <: vec<B> -> nok, need bound on B
  expect_A($b); // ok: B <: A -> B <: B -> ok
  expect_B($b); // ok: B <: B -> ok
  expect_vecA($b); // nok: B <: vec<A> -> nok, need bound on B
  expect_vecB($b); // nok: B <: vec<B> -> nok, need bound on B
  expect_A(
    $v,
  ); // ok: vec<nothing> <: A -> vec<nothing> <: B -> vec<nothing> <: vec<A> -> nothing <: A -> ok
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
