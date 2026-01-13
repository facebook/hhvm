<?hh
<<file: __EnableUnstableFeatures('case_types', 'recursive_case_types')>>

case type A = vec<A> | int;

function expect_A(A $_): void {}
function expect_vecA(vec<A> $_): void {}

function foo(
  A $a,
  int $i,
  vec<A> $va,
  vec<int> $vi,
  vec<nothing> $v,
  vec<vec<nothing>> $vv,
  vec<vec<int>> $vvi,
  vec<vec<A>> $vva,
): void {
  expect_A($a);
  expect_vecA($a); // nok: A <: vec<A> -> nok, need bound on A
  expect_A($i);
  expect_A($va);
  expect_A($vi);
  expect_A($v);
  expect_A($vv);
  expect_A($vvi);
  expect_A($vva);
}
