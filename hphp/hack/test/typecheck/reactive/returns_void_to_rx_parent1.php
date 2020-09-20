<?hh // partial

class Foo {
  <<__Rx, __ReturnsVoidToRx>>
  public function test(): this {
    return $this;
  }
}

class Bar extends Foo {
  <<__Rx, __ReturnsVoidToRx>>
  public function test(): this {
    parent::test();
    return $this;
  }
}
