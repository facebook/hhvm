<?hh // strict
class A {
  public ?bool $foo;

  public function setFoo(): this {
    $this->foo = true;
    return $this;
  }

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
