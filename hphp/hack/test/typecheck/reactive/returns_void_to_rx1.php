<?hh // strict

class A {
  public ?bool $foo;
  // OK because of __ReturnsVoidToRx
  <<__Rx, __Mutable, __ReturnsVoidToRx>>
  public function setFoo(): this {
    $this->foo = true;
    return $this;
  }
  // OK because of __ReturnsVoidToRx
  <<__Rx, __ReturnsVoidToRx>>
  public static function setBar(<<__Mutable>>A $a): A {
    $a->foo = false;
    return $a;
  }
}
