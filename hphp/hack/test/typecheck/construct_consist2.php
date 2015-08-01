<?hh

<<__ConsistentConstruct>>
class C1 {}

class C2 extends C1 {
  // optional arg is ok
  public function __construct(?A $a = null) {}
}

class C3 extends C1 {
  // another default constructor is OK
}
