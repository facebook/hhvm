<?hh
<<file: __EnableUnstableFeatures('case_types', 'recursive_case_types')>>

case type A = B | int;

case type B = A;

function expect_A(A $x): void {}
function expect_B(B $x): void {}

function foo(A $a, B $b, int $i): void {
  expect_A($a);
  expect_B($a);
  expect_A($b);
  expect_B($b);
  expect_A($i);
  expect_B($i);
}
