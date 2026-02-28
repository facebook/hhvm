<?hh

class TheParent {}

class A extends TheParent {
  /*range-start*/
  public function foo(): void {
    400 + 8;
  }
  public function bar(): void {
    400 + 8;
  }
  /*range-end*/
}
