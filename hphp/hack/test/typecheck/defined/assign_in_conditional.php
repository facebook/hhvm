<?hh

function f(bool $b): void {
  if ($b && ($x = 1) !== 0) {
    expect_int($x); // ok
  }
  expect_int($x); // error
}

function expect_int(int $x): void {}
