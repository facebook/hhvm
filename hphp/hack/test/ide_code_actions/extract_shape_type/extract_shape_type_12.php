<?hh

class A {
  public function foo(int $x): void {
    /*range-start*/
    $x = shape('x' => 1);
    /*range-end*/
  }
}
