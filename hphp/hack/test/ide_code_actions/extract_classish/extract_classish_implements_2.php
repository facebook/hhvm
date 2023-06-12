<?hh

class TheParent {}

interface I {}
interface J {}
class A extends TheParent
 implements I, J {
  /*range-start*/
  public function foo(): void {
    400 + 8;
  }
  /*range-end*/
}
