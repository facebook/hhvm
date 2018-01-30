<?hh // strict

class C {
  public function foo(): int {
    return 4;
  }
}

function expect_variadic((function(C...): int) $f): void {
  $f(new C());
}

// In all these tests we're calling a method on the variadic instead of
// accessing an element and calling a method on that, e.g. $y[0]->foo()
function test(): void {
  expect_variadic((...$y) ==> $y->foo());
  expect_variadic((C ...$y) ==> $y->foo());

  $lambda = (...$y) ==> $y->foo();
  $lambda = (C ...$y) ==> $y->foo();
}
