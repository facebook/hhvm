<?hh

interface C {
  <<__DynamicallyCallable>>
  public function foo(): void;
}

class D implements C {
  public function foo(): void {}
}
