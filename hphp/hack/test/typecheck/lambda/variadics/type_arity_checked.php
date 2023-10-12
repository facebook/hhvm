<?hh // strict

class A<T> {}

// must detect that A is missing a type argument
function test(A ... $y): void {}

class B {
  // must detect that A is missing a type argument
  public function test(A ...$y): void {}
}
