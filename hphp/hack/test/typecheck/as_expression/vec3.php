<?hh

function f(Container<string> $x): void {
  $y = $x ?as vec<_>;
  expect_vec1($x); // error
  expect_vec2($x); // error
  expect_vec1($y); // ok
  expect_vec2($y); // ok
}

function expect_vec1(?vec<mixed> $vec): void {}
function expect_vec2(?vec<string> $vec): void {}
