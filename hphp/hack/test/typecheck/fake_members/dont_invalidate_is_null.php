<?hh

class A {
  public ?int $y;
}

function f(A $x, ?A $y): void {
  if (!is_null($x->y) && !is_null($y)) {
    expect_nonnull($x->y);
  }
}

function expect_nonnull(nonnull $x): void {}
