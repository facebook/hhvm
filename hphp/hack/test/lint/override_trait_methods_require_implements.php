<?hh

interface MyInterface {
  public function foo(): int;
}

trait MyTrait {
  require implements MyOtherInterface;

  public function bar(): int { return $this->foo(); }
}

class MyClass implements MyInterface {
  // This trait is useless, we've overridden all its methods.
  use MyTrait;

  public function foo(): int { return 1; }

  <<__Override>>
  public function bar(): int { return 2; }
}
