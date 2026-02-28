<?hh

class Klass {
  public function foo(int $x): void {
      /*range-start*/
      $y = $x;
      yield 1;
      /*range-end*/
      $z = $y;
  }
}
