<?hh

final class CGeneric<T> {
  public function m<Tm>(T $x, Tm $y): void {}

  public function f(T $x): void {
    $this->m($x, 0.0);
//         ^ hover-at-caret
  }
}
