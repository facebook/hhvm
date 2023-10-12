<?hh // strict

function expect_fn((function(int, string, string, int...): int) $f): void {
  $f(1, "hello", "world");
}

function expect_fn2((function(int): int) $f): void {
  $f(1);
}

function test(): void {
  /* These will fail as ...$y will cover the string, string, int... parameters
   * when passed to expect_fn while $lambda assumes ...$y will only have num
   * due to numerical operation.
   */
  $lambda = ($a, ...$y) ==> {
    $c = ($y[0].$y[1]);
    return 1;
  };
  expect_fn($lambda);

  expect_fn(($a, ...$y) ==> {
    $c = ($y[0] + $y[1]);
    return 1;
  });

  // This should work
  expect_fn(($a, ...$y) ==> {
    return 1;
  });

  // Error
  expect_fn(($a, ...$y) ==> {
    $c = $y[0]->bar();
    return 1;
  });

  // Error, no bar() on string
  expect_fn2(($a, string ...$y) ==> {
    $c = $y[0]->bar();
    return 1;
  });

  // Error, should have variadic arg
  expect_fn(($a, $b, $c) ==> {
    return 1;
  });

  // Error
  expect_fn(($a, $b, $c, $d) ==> {
    return 1;
  });
}
