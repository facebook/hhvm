<?hh

class Base {
  public function foo(): void {}
}

class Child extends Base {
  <<__NeedsConcrete>>
  public function foo(): void {}
}
