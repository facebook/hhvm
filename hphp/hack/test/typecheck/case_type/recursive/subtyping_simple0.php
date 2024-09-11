<?hh
<<file: __EnableUnstableFeatures('case_types')>>

case type A as A = int;

function expect_A(A $_): void {}
function expect_string(string $_): void {}
function expect_int(int $_): void {}

function foo(A $a, int $i, string $s): void {
  expect_A($a);
  expect_string($a); // nok: A <: string -> A <: string -> recursing -> nok
  expect_int($a); // nok: A <: int -> A <: int -> recursing -> nok
  expect_A($s); // nok: string <: A -> string <: A -> recursing -> nok
  expect_A($i); // ok: int <: A -> int <: int -> ok
}
