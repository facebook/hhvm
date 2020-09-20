<?hh

class C {
  <<__DynamicallyCallable>>
  public function foo(): void {}
}

trait MyTrait {
  <<__Override>>
  public function foo(): void {}
}

class D extends C {
  use MyTrait;
}
