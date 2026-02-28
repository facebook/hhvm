<?hh

class A {
  /*range-start*/
  public function foo(): void {
    400 + 8;
  }
  public int $x;
  private function bar(): void {}
  public function baz(): int {
    return 400 + 8;
  }
  /*range-end*/
}
