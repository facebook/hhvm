<?hh
<<file: __EnableUnstableFeatures('case_types', 'recursive_case_types')>>

case type A = A | int;

function expect_A(A $_): void {}
function expect_int(int $_): void {}

function foo(A $a, int $i): void {
  expect_A($a);
  expect_int($a); // nok: A <: int -> nok, need bound on A
  expect_A($i); // ok: int <: A -> int <: A | int -> ok
}
