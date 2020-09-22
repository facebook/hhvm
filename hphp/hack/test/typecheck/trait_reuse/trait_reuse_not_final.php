<?hh

trait MyTrait {
  public function foo(): void {}
}


class MyBase {
  use MyTrait;
}


class MyClass extends MyBase {
  use MyTrait;
}
