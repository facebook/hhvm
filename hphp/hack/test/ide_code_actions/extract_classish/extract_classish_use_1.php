<?hh

trait Tr {}

class A {
  use Tr;
  /*range-start*/
  public function foo(): void {
    400 + 8;
  }
  public int $x;
  /*range-end*/
}
