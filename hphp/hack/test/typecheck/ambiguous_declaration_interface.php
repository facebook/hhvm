<?hh // partial
interface I1 {
  public function foo(): int;
}
interface I2 {
  public function foo(): num;
}

interface I3 extends I1, I2 {}
