<?hh

class A {
  public ?int $y;
}

function f(A $x, ?A $y): void {
  if (!($x->y is null) && !($y is null)) {
    expect_nonnull($x->y);
  }
}

function expect_nonnull(nonnull $x): void {}
