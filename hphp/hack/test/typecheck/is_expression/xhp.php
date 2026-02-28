<?hh

final class :my-xhp {}

function f(mixed $x): void {
  if ($x is :my-xhp) {
    expect_xhp($x);
  }
}

function expect_xhp(:my-xhp $x): void {}
