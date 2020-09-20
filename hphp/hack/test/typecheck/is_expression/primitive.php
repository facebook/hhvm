<?hh // strict

function main(mixed $x): void {
  if ($x is bool) {
    expect_bool($x);
  } else if ($x is float) {
    expect_float($x);
  } else if ($x is int) {
    expect_int($x);
  } else if ($x is resource) {
    expect_resource($x);
  } else if ($x is string) {
    expect_string($x);
  }
  expect_bool($x);
  expect_float($x);
  expect_int($x);
  expect_resource($x);
  expect_string($x);
}

function expect_bool(bool $x): void {}
function expect_float(float $x): void {}
function expect_int(int $x): void {}
function expect_resource(resource $x): void {}
function expect_string(string $x): void {}
