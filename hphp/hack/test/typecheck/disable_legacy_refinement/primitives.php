<?hh // strict

function f(mixed $x): void {
  if (is_bool($x)) {
    expect_bool($x);
  }
  if (is_float($x)) {
    expect_float($x);
  }
  if (is_int($x)) {
    expect_int($x);
  }
  if (is_string($x)) {
    expect_string($x);
  }
}

function expect_bool(bool $x): void {}
function expect_float(float $x): void {}
function expect_int(int $x): void {}
function expect_string(string $x): void {}
