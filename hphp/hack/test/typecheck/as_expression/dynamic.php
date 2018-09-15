<?hh // strict

function f(int $x): void {
  expect_int($x as dynamic); // error
  expect_int(42 as dynamic); // error
  expect_dynamic($x as dynamic); // ok
  expect_dynamic(42 as dynamic); // ok
}

function g(int $x): void {
  $x as dynamic;
  expect_int($x); // error
  expect_dynamic($x); // ok
}

function expect_dynamic(dynamic $x): void {}
function expect_int(int $x): void {}
