<?hh
<<file: __EnableUnstableFeatures('case_types', 'recursive_case_types')>>

case type A = B | int;

case type B = vec<A>;

function expect_A(A $x): void {}
function expect_B(B $x): void {}

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
  expect_B($a); // wrong, int is not subtype of B
  expect_A($b);
  expect_B($b);
  expect_A($i);
  expect_B($i); // wrong, int is not subtype of B
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
  expect_A($vs); // wrong
  expect_B($vs); // wrong
}
