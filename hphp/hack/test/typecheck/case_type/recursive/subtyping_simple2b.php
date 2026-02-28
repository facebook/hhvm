<?hh
<<file: __EnableUnstableFeatures('case_types', 'recursive_case_types')>>

case type A as int = A
  | int; // A | int <: int -> A <: int && int <: int -> A <: int -> recursing -> ?

function expect_A(A $_): void {}
function expect_int(int $_): void {}

function foo(A $a, int $i): void {
  expect_A($a);
  expect_int($a); // ok: A <: int -> int <: int -> ok
  expect_A($i); // ok: int <: A -> int <: A | int -> ok
}
