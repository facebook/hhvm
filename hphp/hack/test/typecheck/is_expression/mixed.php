<?hh

function main(int $x): void {
  if ($x is mixed) {
    expect_int($x);
    expect_mixed($x);
  }
}

function expect_int(int $x): void {}
function expect_mixed(mixed $x): void {}
