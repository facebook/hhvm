<?hh

<<__ConsistentConstruct>>
class C1 {}

class C2 extends C1 {
  <<__UNSAFE_Construct>> // explicitly inconsistent with C1
  public function __construct(A $a) {}
}

class C3 extends C2 {
  // still has to be consistent, but now with C2!
  public function __construct() {
    parent::__construct(new A());
  }
}
