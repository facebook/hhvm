<?hh // strict

abstract class C {
  public function foo(): int { return 1; }
}

function test<T as C>(T $x): int {
  return $x->foo();
}
