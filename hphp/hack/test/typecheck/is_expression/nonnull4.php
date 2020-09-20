<?hh // strict

function f(null $x): void {
  if ($x is nonnull) {
    expect_int($x);
    expect_null($x);
    expect_nstring($x);
    expect_bool($x);
  }
}

function expect_int(int $x): void {}
function expect_null(null $x): void {}
function expect_nstring(?string $x): void {}
function expect_bool(bool $x): void {}
