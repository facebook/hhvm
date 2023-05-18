<?hh

class C {
  public function f(string $s): void {
    C::$s();
    //^ hover-at-caret
  }
}
