<?hh // strict

interface I1 {
  abstract public function foo(): mixed;
}

interface I2 {
  abstract public function foo(): int;
}

abstract class C implements I1, I2 {}
