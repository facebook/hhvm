<?hh // strict

// All happy paths

class D {
  public function __construct(private string $item) {}
  public function bar(): int {
    return 4;
  }
}

function expect_variadic((function(D...): int) $f): void {
  $f(new D("first"));
}

function expect_non_variadic((function(D): int) $f): void {
  $f(new D("first"));
}

function expect_nothing((function(): int) $f): void {
  $f();
}

function test1(): void {
  expect_variadic(
    (...$y) ==> {
      return $y[0]->bar();
    },
  );

  expect_variadic(
    (D ...$y) ==> {
      return $y[0]->bar();
    },
  );

  expect_variadic((...$y) ==> $y[0]->bar());

  expect_variadic((D ...$y) ==> $y[0]->bar());

  $lambda1 = (...$y) ==> $y[0]->bar();
  expect_variadic($lambda1);

  $lambda2 = (D ...$y) ==> $y[0]->bar();
  expect_variadic($lambda2);

  expect_non_variadic((...$y) ==> $y[0]->bar());

  $lambda3 = (...$y) ==> $y[0]->bar();
  expect_non_variadic($lambda3);
}

function test2(): void {
  // These will typecheck but will cause runtime errors (as ...$y will be empty)
  expect_nothing(
    function(D ...$y) {
      return $y[0]->bar();
    },
  );
  expect_nothing(
    function(...$y) {
      return $y[0]->bar();
    },
  );
  expect_nothing(
    (...$y) ==> {
      return $y[0]->bar();
    },
  );
  expect_nothing(
    (D ...$y) ==> {
      return $y[0]->bar();
    },
  );
  expect_nothing((...$y) ==> $y[0]->bar());
  expect_nothing((D ...$y) ==> $y[0]->bar());
}
