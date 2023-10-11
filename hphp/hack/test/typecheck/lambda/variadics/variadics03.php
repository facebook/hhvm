<?hh

// All happy paths

class C {
  public function foo(): int {
    return 3;
  }
}

class D extends C {
  public function __construct(private string $item) {}
  public function bar(): int {
    return 4;
  }
}

function expect_D((function(D...): int) $f): void {
  $f(new D("first"));
}

// Check for contravariance
function test(): void {
  expect_D((C ...$y) ==> $y[0]->foo());

  $lambda = (C ...$y) ==> $y[0]->foo();
  expect_D($lambda);
}
