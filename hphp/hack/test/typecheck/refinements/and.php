<?hh

function f(mixed $x): void {
  if (!$x is int && !$x is string) {

  } else {
    expect_arraykey($x);
    expect_int($x); // error
    expect_string($x); // error
  }
}

function g(arraykey $x, arraykey $y): void {
  if (!$x is string && !$y is string) {
    expect_int($x); // no error
    expect_int($y); // no error
  } else {
    expect_string($x); // error: $x: int, $y: string works
    expect_int($x);    // error: $x: string, $y: int works
    expect_string($y); // error: $x: string, $y: int works
    expect_int($y);    // error: $x: int, $y: string works
  }
}

function h(?int $x): void {
  if ($x !== null && is_zero($x)) { // no error

  }
}

function expect_arraykey(arraykey $x): void {}
function expect_int(int $x): void {}
function expect_string(string $x): void {}
function is_zero(int $x): bool {
  return $x === 0;
}
