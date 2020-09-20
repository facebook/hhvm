<?hh // partial

trait C {
  public function f(): void {}
}

abstract class D {
  use C;
  abstract public function f(): void;
}
