<?hh //strict

class C {
  public function foo(): int {
    return 1;
  }
}

class D {
  public function foo(): int {
    return 1;
  }
}

function expect_C((function(C...): void) $fn): void {
  $fn(new C(), new C());
}

/* These should all error */
function test(C ...$c): void {
  expect_C(
    (D ...$d) ==> {
      $d[0]->foo();
    },
  );

  $lambda = (D ...$d) ==> {
    $d[0]->foo();
  };
  expect_C($lambda);

  $lambda(...$c);

  $lambda(new C(), new C());
}
