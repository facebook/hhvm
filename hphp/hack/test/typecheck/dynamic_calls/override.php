<?hh

class C {
  <<__DynamicallyCallable>>
  public function foo(): void {}
}

class D extends C {
  <<__Override>>
  public function foo(): void {}
}
