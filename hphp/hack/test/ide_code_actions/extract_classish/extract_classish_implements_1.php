<?hh

class TheParent {}

interface I {}
class A extends TheParent
 implements I {
  /*range-start*/
  public function foo(): void {
    400 + 8;
  }
  /*range-end*/
}
