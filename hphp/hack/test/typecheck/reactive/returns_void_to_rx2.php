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

<<__Rx>>
function f(): void {
  $a = \HH\Rx\mutable(new A());
  // ERROR because of __ReturnsVoidToRx
  $a->setFoo()
    ->bar();
}
