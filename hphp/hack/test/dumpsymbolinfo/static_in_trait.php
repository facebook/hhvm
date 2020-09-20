<?hh // strict

interface A {}

<<__ConsistentConstruct>>
trait X implements A {
  public function foo(): A {
    return new static();
  }
}

class Y {
  use X;

  public function buildAnObjectAFromTrait(): A {
    return $this->foo();
  }
}
