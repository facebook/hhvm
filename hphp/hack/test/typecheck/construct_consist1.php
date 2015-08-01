<?hh

<<__ConsistentConstruct>>
class C1 {}

class C2 extends C1 {
  // new mandatory arg is not ok
  public function __construct(A $a) {}
}
