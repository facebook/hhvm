<?hh
<<file: __EnableUnstableFeatures('case_types', 'recursive_case_types')>>

case type A = B | int;

case type B as vec<A> = vec<A>;

function expect_A(A $x): void {}
function expect_B(B $x): void {}
function expect_vecA(vec<A> $x): void {}
function expect_vecB(vec<B> $x): void {}

function foo(
  A $a,
  B $b,
  int $i,
  vec<A> $va,
  vec<int> $vi,
  vec<B> $vb,
  vec<vec<int>> $vvi,
  vec<vec<A>> $vva,
  vec<vec<B>> $vvb,
  vec<string> $vs,
): void {
  expect_A($a);
  expect_B($a); // nok: A <: B -> nok, need constraint on A
  expect_vecA($a); // nok: A <: vec<A> -> nok, need constraint on A
  expect_vecB($a); // nok: A <: vec<B> -> nok, need constraint on A
  expect_A($b); // ok: B <: A -> B <: B -> ok
  expect_B($b);
  expect_vecA($b); // ok: B <: vec<A> -> vec<A> <: vec<A> -> ok
  expect_vecB(
    $b,
  ); // nok: B <: vec<B> -> vec<A> <: vec<B> -> A <: B -> nok, need constraint on A
  expect_A($i);
  expect_B($i); // nok: int <: B -> int <: vec<A> -> nok
  expect_A($va);
  expect_B($va);
  expect_A($vi);
  expect_B($vi);
  expect_vecA($vi); // ok: vec<int> <: vec<A> -> int <: A -> ok
  expect_vecB(
    $vi,
  ); // nok: vec<int> <: vec<B> -> int <: B -> int <: vec<A> -> nok
  expect_A($vb);
  expect_B($vb);
  expect_A($vvi);
  expect_B($vvi);
  expect_A($vva);
  expect_B($vva);
  expect_A($vvb);
  expect_B($vvb);
  expect_A(
    $vs,
  ); // nok: string <: A -> string <: B | int -> string <: B -> string <: vec<A> -> nok
  expect_B($vs); // nok
}
