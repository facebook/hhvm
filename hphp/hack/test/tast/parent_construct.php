<?hh

class B {
  public function __construct(num $x) {}
}
class A extends B {
  public function __construct(int $x) {
    parent::__construct($x);
  }
}
