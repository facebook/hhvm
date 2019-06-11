<?hh // strict

class C {
  public function foo(): int {
    return 4;
  }
}

class D extends C {
  public function bar(): int {
    return 4;
  }
}

function expect_variadic((function(D, C, int): int) $f): void {
  $f(new D(), new C(), 1);
}

function test(): void {
  //OK
  expect_variadic(
    (...$y) ==> {
      $x = $y[0];
      if ($x is C) {
        return $x->foo();
      }
      if ($x is int) {
        return $x;
      }
      return 0;
    },
  );

  //Error, can only use num (int/float) in numerical operations.
  expect_variadic(
    (...$y) ==> {
      return $y[0] + $y[1];
    },
  );

  expect_variadic((...$y) ==> $y[0]->foo()); //Error, no foo() on int.

  $lambda = (...$y) ==> $y[0]->foo();
  expect_variadic($lambda); //Error, no foo() on int.

  $lambda2 = (C ...$y) ==> $y[0]->foo();
  expect_variadic($lambda2); //Error, incorrect variadic type
}
