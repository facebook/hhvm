<?hh
<<file: __EnableUnstableFeatures('case_types', 'recursive_case_types')>>

case type A as A = A;

function expect_A(A $_): void {}
function expect_string(string $_): void {}

function foo(A $a, string $s): void {
  expect_A($a);
  expect_string($a); // nok: A <: string -> A <: string -> recursing -> nok
  expect_A($s); // nok: string <: A -> string <: A -> recursing -> nok
}
