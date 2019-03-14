<?hh // partial

/**
 * TODO(T29281383)
 */

function f1($x): void {
  expect_int($x as int);
}

function f2($x): void {
  expect_string($x as int);
}

function f3($x): void {
  expect_resource($x as int);
}

function g($x): void {
  $x as int;
  expect_int($x);
  expect_string($x);
  expect_resource($x);
}

function expect_int(int $x): void {}
function expect_string(string $x): void {}
function expect_resource(resource $x): void {}
