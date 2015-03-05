<?hh // strict

interface I {
  abstract const type T;

  public function foo(this::T $t): void;
}

class Incompat implements I {
  const type T = mixed;

  // If an interface parameter is defined in terms of a type constant then we
  // require implementers to provide a type hint
  public function foo($t): void {}
}
