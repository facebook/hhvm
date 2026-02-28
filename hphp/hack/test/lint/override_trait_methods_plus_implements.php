<?hh

interface MyOtherInterface {}

interface MyInterface {
  public function foo(): int;
}

trait MyTrait implements MyInterface {
  require implements MyOtherInterface;
  public function foo(): int { return 1; }
}

class MyClass {
  // We've overridden the method, but we still need the trait because
  // it includes `implements MyInterface`.
  use MyTrait;

  <<__Override>>
  public function foo(): int { return 2; }
}
