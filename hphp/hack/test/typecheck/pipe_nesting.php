<?hh // strict

// This test cases checks for a previous bug that
// $$ loses its typing information in a ternary
// expression. In this bug case the $$ will have Tany,
// hence passing both checks. Had it been properly typed,
// this will report a type error at expect_int
function expect_success(bool $b): void {
  $mystring = 1
    |> $$ ? (i2b($$) |> b2s($$)) : (i2s($$) |> s2s($$));
  expect_string($mystring);
}

function expect_success2(bool $b): void {
  $mystring = $b
    |> $$ ? (b2s($$) |> $$) : ($$ |> b2s($$));
  expect_string($mystring);
}

function i2s(int $i): string {
  return '42';
}

function i2b(int $i): bool {
  return true;
}

function b2s(bool $b): string {
  return 'foo';
}

function s2s(string $s): string {
  return $s.$s;
}

function expect_string(string $s): void {}
