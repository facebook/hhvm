<?hh

function f(bool $b): arraykey {
  $x = $b ? 1 : "";
  expect_int($x); // error
  expect_string($x); // error
  return $x;
}

function expect_int(int $x): void {}
function expect_string(string $x): void {}
