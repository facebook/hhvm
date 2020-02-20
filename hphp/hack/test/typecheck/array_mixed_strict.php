<?hh // strict

interface I {}
class C implements I {}
class D implements I {}

class Q {
  public array<I> $arr;
  public function __construct() {
    $this->arr = varray[new C(), new D()];
  }
}
