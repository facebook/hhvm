<?hh

class ABC {
  const type T = int;
  public function taccess_this(this::T $x): void {}
}
