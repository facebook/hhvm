<?hh // strict

function f(bool $b, string $x): void {
  if ($b && ($x = 1) !== 0) {
    expect_int($x); // ok
  } else {
    expect_int($x); // error: if $b is false, $x is a string
    expect_string($x); // error: if $b is true, $x is an int
  }
  expect_int($x); // error
  expect_string($x); // error
}

function g(bool $b, string $x): void {
  if ($b || ($x = 1) !== 0) {
    expect_int($x); // error: if $b is true, $x is a string
    expect_string($x); // error: if $b is false, $x is an int
  } else {
    expect_int($x); // ok: $b is necessarily false, so $x is an int
  }
  expect_int($x); // error
  expect_string($x); // error
}

function expect_int(int $x): void {}
function expect_string(string $x): void {}
