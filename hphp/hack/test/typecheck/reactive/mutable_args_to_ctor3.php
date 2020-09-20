<?hh // partial

<<__ConsistentConstruct>>
class A {
  <<__Rx>>
  public function __construct(this $a) {
  }

  <<__Rx, __Mutable>>
  public function make(): this {
    // ERROR, passing mutable as immutable
    return new static($this);
  }
}
