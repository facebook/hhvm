<?hh // strict
class A {
  public function nonrx(): int {
    return 42;
  }

  public function rxlocal(): int {
    return $this->nonrx();
  }
}
