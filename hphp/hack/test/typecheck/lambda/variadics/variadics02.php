<?hh

// All happy paths

class D {
  public function __construct(private string $item) {}
  public function bar(): int {
    return 4;
  }
}

function expect_multiple_params_trailing_variadic(
  (function(int, string, D, D...): int) $f,
): void {
  $f(1, "hello", new D("first"), new D("second"));
}

// Inferred types
function test1(): void {
  $lambda = ($i, $str, $d, ...$d_var) ==> {
    if ($str == "foo") {
      return $d_var[0]->bar();
    }
    return 0;
  };
  expect_multiple_params_trailing_variadic($lambda);

  // Note: Variadic $d_var covers both D and D... in
  // expect_multiple_params_trailing_variadic
  $lambda2 = ($i, $str, ...$d_var) ==> {
    if ($str == "foo") {
      return $d_var[0]->bar();
    }
    return 0;
  };
  expect_multiple_params_trailing_variadic($lambda2);

  expect_multiple_params_trailing_variadic(
    ($i, $str, $d, ...$d_var) ==> {
      if ($str == "foo") {
        return $d_var[0]->bar();
      }
      return 0;
    },
  );

  // Note: Variadic $d_var covers both D and D... in
  // expect_multiple_params_trailing_variadic
  expect_multiple_params_trailing_variadic(
    ($i, $str, ...$d_var) ==> {
      if ($str == "foo") {
        return $d_var[0]->bar();
      }
      return 0;
    },
  );
}

// Explicit types
function test2(): void {
  $lambda = (int $i, string $str, D $d, D ...$d_var) ==> {
    if ($str == "foo") {
      return $d_var[0]->bar();
    }
    return 0;
  };
  expect_multiple_params_trailing_variadic($lambda);

  // Note: Variadic $d_var covers both D and D... in
  // expect_multiple_params_trailing_variadic
  $lambda2 = (int $i, string $str, D ...$d_var) ==> {
    if ($str == "foo") {
      return $d_var[0]->bar();
    }
    return 0;
  };
  expect_multiple_params_trailing_variadic($lambda2);

  expect_multiple_params_trailing_variadic(
    (int $i, string $str, D $d, D ...$d_var) ==> {
      if ($str == "foo") {
        return $d_var[0]->bar();
      }
      return 0;
    },
  );

  // Note: Variadic $d_var covers both D and D... in
  // expect_multiple_params_trailing_variadic
  expect_multiple_params_trailing_variadic(
    (int $i, string $str, D ...$d_var) ==> {
      if ($str == "foo") {
        return $d_var[0]->bar();
      }
      return 0;
    },
  );
}
