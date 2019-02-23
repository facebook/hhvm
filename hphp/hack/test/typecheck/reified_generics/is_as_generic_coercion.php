<?hh // strict

class C<<<__Enforceable>> reify T> {
  public function gate_as(mixed $x): void {
    $this->f($x as T);
  }

  public function gate_is(mixed $x): void {
    if ($x is T) {
        $this->f($x);
    }

    // here, $x is a (mixed | T), error
    $this->f($x);
  }

  public function f(T $t): void {}
}
