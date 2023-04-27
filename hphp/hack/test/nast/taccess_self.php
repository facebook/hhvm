<?hh

class ABC {
  const type T = int;
  public function taccess_self(self::T $x): void {}
}
