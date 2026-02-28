<?hh

interface I1 {
  public function foo(): mixed;
}

interface I2 {
  public function foo(): int;
}

abstract class C implements I1, I2 {}
