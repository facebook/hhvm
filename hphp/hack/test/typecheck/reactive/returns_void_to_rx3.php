<?hh // strict

class A {
  public ?bool $foo;
  <<__Rx, __Mutable, __ReturnsVoidToRx>>
  public function setFoo(): this {
    $this->foo = true;
    return $this;
  }
  <<__Rx>>
  public function bar(): string {
    return "";
  }
}

function f(): void {
  $a = new A();
  // OK
  $a->setFoo()
    ->bar();
}
