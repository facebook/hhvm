<?hh

class C {
  public function foo(): void {
    $x =/*range-start*/ shape() /*range-end*/;
  }
}
