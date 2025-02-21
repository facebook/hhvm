<?hh

final class CGeneric<T> {
  public ?Vector<T> $v;

  public function f(T $x): void {
    $this->v;
//         ^ hover-at-caret
  }
}
