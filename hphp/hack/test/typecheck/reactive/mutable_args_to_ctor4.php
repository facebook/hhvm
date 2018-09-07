<?hh

<<__ConsistentConstruct>>
class A {
  <<__Rx>>
  public function __construct(<<__Mutable>>this $a) {
  }

  <<_Rx>>
  public function make(): this {
    // OK, passing mutable as mutable
    return new static($this);
  }
}
