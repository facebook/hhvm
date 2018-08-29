<?hh //strict

function f(): void {
  if ($y = g()) {
    expect_int($y);
  }
}

function h(?int $x): void {
  if ($y = $x) {
    expect_int($x);
    expect_int($y);
  }
}

function f2(): void {
  if (($y = g()) !== null) {
    expect_int($y);
  }
}

function h2(?int $x): void {
  if (($y = $x) !== null) {
    expect_int($x);
    expect_int($y);
  }
}

function g(): ?int {
  return null;
}

function expect_int(int $x): void {}
