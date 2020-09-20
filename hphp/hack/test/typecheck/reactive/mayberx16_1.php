<?hh // strict

interface A {
  <<__Rx>>
  public function f(Rx<(function(): int)> $f): int;
}

interface B extends A {
  // OK - it is safe since matching parameter in base class
  // is completely reactive
  <<__Rx, __AtMostRxAsArgs>>
  public function f(<<__AtMostRxAsFunc>>(function(): int) $f): int;
}
