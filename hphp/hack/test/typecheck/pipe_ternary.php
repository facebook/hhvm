<?hh

// This test cases checks for a previous bug that
// $$ loses its typing information in a ternary
// expression. In this bug case the $$ will have Tany,
// hence passing both checks. Had it been properly typed,
// this will report a type error at expect_string
function expect_fail(bool $b): void {
  $broken = 1 |> ($b ? $$ : $$);
  expect_int($broken);
  expect_string($broken);
}

function expect_int(int $i): void {}
function expect_string(string $s): void {}
