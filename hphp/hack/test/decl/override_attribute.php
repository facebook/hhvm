<?hh

class B {
  public function f(): void {}
}

class C extends B {
  <<__Override>>
  public function f(): void {}
}
