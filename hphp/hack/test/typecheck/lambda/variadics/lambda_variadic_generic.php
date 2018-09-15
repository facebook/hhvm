<?hh // strict

class C {
  public function foo(): int {
    return 3;
  }
}

class A<T> {
  public function __construct(private T $value) {}
  public function foobar(): T {
    return $this->value;
  }
}

function expect_Generic((function(A<C>...): C) $f): void {
  $f(new A(new C()));
}

// Check for contravariance
function test(): void {
  $lambda = (A<C> ...$y) ==> $y[0]->foobar()->foo(); //OK

  expect_Generic((A<C> ...$y) ==> {
    $c = $y[0]->foobar();
    $c->foo();
    return $c;
  }); //OK

  expect_Generic((...$y) ==> $y[0]->foobar()); //OK

  expect_Generic((...$y) ==> {
    $c = $y[0]->foobar();
    $c->foo();
    return $c;
  }); //OK

  expect_Generic((A ...$y) ==> $y[0]->foobar()); //Error

  expect_Generic((...$y) ==> $y[0]->foobar()->bar()); //Error
}
