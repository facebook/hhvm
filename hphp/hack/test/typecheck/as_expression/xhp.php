<?hh // strict

final class :my-xhp {}

function f(mixed $x): void {
  expect_xhp($x as :my-xhp);
}

function expect_xhp(:my-xhp $x): void {}
