<?hh

interface IFoo {
  public function implement_me();
}

interface IBar {}

interface IQux extends IFoo, IBar {}

class Foo implements IFoo {
  public function implement_me() {}
  public function override_me() {}
}

class Bar extends Foo implements IQux {
  public function not_overridden() {}
}

class Baz extends Bar {
  public function override_me() {}
}
