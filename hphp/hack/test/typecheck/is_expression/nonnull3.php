<?hh

function f(bool $option): void {
  $x = $option ? return_int() : return_nstring();
  if ($x is nonnull) {
    expect_nonnull($x);
    expect_arraykey($x);
  }
}

function return_int(): int {
  return 1;
}
function return_nstring(): ?string {
  return 'foo';
}

function expect_nonnull(nonnull $x): void {}
function expect_arraykey(arraykey $x): void {}
