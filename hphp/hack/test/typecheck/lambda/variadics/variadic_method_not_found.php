<?hh // strict

class D {
  public function __construct(private string $item) {}
  public function bar(): int {
    return 4;
  }
}

function expect_variadic((function(D...): int) $f): void {
  $f(new D("first"));
}

// Method foo not found
// These should all cause errors
function test(): void {
  expect_variadic(
    function(...$y): int {
      return $y[0]->foo();
    },
  );

  expect_variadic(
    function(D ...$y): int {
      return $y[0]->foo();
    },
  );

  expect_variadic((...$y) ==> $y[0]->foo());

  expect_variadic((D ...$y) ==> $y[0]->foo());

  $lambda = (...$y) ==> $y[0]->foo();
  expect_variadic($lambda);

  $lambda2 = (D ...$y) ==> $y[0]->foo();

  $closure1 = function(...$y): int {
    return $y[0]->foo();
  };
  expect_variadic($closure1);

  $closure2 = function(D ...$y): int {
    return $y[0]->foo();
  };
}
