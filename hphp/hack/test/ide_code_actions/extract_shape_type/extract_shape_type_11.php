<?hh

class A {
  public function foo(int $x): void {
    /*range-start*/
    $x = shape('x' => 1);
    $y = shape('y' => 1);
    /*range-end*/
  }
}
