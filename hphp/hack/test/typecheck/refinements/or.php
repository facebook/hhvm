<?hh // strict

function f(mixed $x): void {
  if ($x is string || $x is int) {
    expect_arraykey($x);
    expect_int($x); // error
    expect_string($x); // error
  }
}

function g(mixed $x, mixed $y): void {
  if ($x is string || $y is string) {
    expect_string($x); // error
    expect_string($y); // error
  }
}

function h(?int $x): void {
  if ($x === null || is_zero($x)) { // no error

  }
}

function expect_arraykey(arraykey $x): void {}
function expect_int(int $x): void {}
function expect_string(string $x): void {}
function is_zero(int $x): bool {
  return $x === 0;
}
