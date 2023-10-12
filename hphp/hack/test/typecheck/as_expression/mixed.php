<?hh // strict

function f(int $x): void {
  expect_int($x as mixed);
  expect_int(42 as mixed);
}

function g(int $x): void {
  $x as mixed;
  expect_int($x);
}

function expect_int(int $x): void {}
