<?hh // partial

function f(?int $x): void {
  if ($x <> null) {
    expect_int($x);
  }
}

function expect_int(int $y): void {}
