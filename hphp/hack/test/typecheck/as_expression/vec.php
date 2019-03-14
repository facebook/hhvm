<?hh // partial

function f(mixed $x): void {
  $x as vec;
  expect_vec1($x); // ok
  expect_vec2($x); // error
}

function g(mixed $x): void {
  $x as vec<_>;
  expect_vec1($x); // ok
  expect_vec2($x); // error
}

function h(Container<string> $x): void {
  $x as vec<_>;
  expect_vec1($x); // ok
  expect_vec2($x); // ok
}

function expect_vec1(vec<mixed> $vec): void {}
function expect_vec2(vec<string> $vec): void {}
