<?hh

class A<reify T as arraykey> {}

function foo(A<string> $x): void {
  $x is A<bool>;
}

class B {
  public function foo(A<string> $x): void {
    (A<B> $x) ==> {};
    $x as A<bool>;
  }
}
