<?hh
<<file: __EnableUnstableFeatures('case_types', 'recursive_case_types')>>

case type A as B = B;

case type B as vec<A> = vec<A>;

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
  expect_A($a); // ok: A <: A
  expect_B($a); // ok: A <: B -> ok with bound on A
  expect_vecA(
    $a,
  ); // ok: A <: vec<A> -> B <: vec<A> -> vec<A> <: vec<A> -> A <: A -> ok
  expect_vecB(
    $a,
  ); // ok: A <: vec<B> -> vec<A> <: vec<B> -> A <: B -> ok with bound on A
  expect_vecvecA(
    $a,
  ); // ok: A <: vec<vec<A>> -> B <: vec<vec<A>> -> vec<A> <: vec<vec<A>> -> A <: vec<A> -> ... -> ok
  expect_vecvecB(
    $a,
  ); /* ok: A <: vec<vec<B>> -> B <: vec<vec<B>> -> vec<A> <: vec<vec<B>>
  -> A <: vec<B> -> B <: vec<B> -> vec<A> <: vec<B> -> A <: B -> B <: B -> ok */
  expect_A($b); // ok: B <: A -> B <: B -> ok
  expect_B($b); // ok: B <: B -> ok
  expect_vecA($b); // ok: B <: vec<A> -> vec<A> <: vec<A> -> A <: A -> ok
  expect_vecB(
    $b,
  ); // ok: B <: vec<B> -> vec<A> <: vec<B> -> A <: B -> ok thanks to bound on A
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
