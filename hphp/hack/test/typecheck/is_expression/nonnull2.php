<?hh

function f(mixed $x): void {
  if ($x is nonnull) {
    expect_nonnull($x);
    expect_int($x);
  }
}

function expect_nonnull(nonnull $x): void {}
function expect_int(int $x): void {}
