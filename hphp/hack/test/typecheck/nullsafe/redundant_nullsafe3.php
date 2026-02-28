<?hh

class A {
  public function foo(): void {}
}

function f(A $x): void {
  $x?->foo();
}
