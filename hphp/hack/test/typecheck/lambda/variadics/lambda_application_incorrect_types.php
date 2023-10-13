<?hh

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

// Lambda application, incorrect types
function test(): void {
  $lambda1 = (...$y) ==> $y[0]->foo(); //OK
  $lambda1(new D()); //OK
  $lambda1(new C()); //OK
  $lambda1("hello"); //Error

  $lambda2 = (C ...$y) ==> $y[0]->foo();
  $lambda2(new D()); //OK
  $lambda2(new C()); //OK
  $lambda2(new C(), new D(), new C()); //OK

  $lambda3 = (D ...$y) ==> $y[0]->foo();
  $lambda3(new D()); //OK
  $lambda3(new C()); //Error

  // OK
  // This should pass as variadic is empty and so its type is irrelevant.
  // This will cause a runtime error instead.
  $lambda4 = ($c, $d, $c2, ...$y) ==> $y[0]->badMethod();
  $lambda4(new C(), new D(), new C());

  // Error: This will fail due to the int
  $lambda5 = (...$y) ==> $y[0]->foo();
  $lambda5(new C(), new D(), new C(), 1);
}
