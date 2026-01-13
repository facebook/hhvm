<?hh
<<file: __EnableUnstableFeatures('case_types', 'recursive_case_types')>>

case type A as string = A; // A is essentially nothing, so technically that's fine

function expect_A(A $_): void {}
function expect_string(string $_): void {}

function foo(A $a, string $s): void {
  expect_A($a);
  expect_string($a); // ok: A <: string -> string <: string -> ok
  expect_A($s); // nok: string <: A -> string <: A -> recursing -> nok
}
