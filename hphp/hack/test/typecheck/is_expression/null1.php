<?hh

function f(?int $x): void {
  if ($x is null) {
    expect_null($x);
    expect_nstring($x);
  } else {
    expect_int($x);
  }
}

function g(?int $x): void {
  if (!$x is null) {
    expect_int($x);
  } else {
    expect_null($x);
    expect_nstring($x);
  }
}

function expect_int(int $x): void {}
function expect_null(null $x): void {}
function expect_nstring(?string $x): void {}
