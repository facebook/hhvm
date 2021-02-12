<?hh
class Foo {

  public function test(): this {
    return $this;
  }
}

class Bar extends Foo {

  public function test(): this {
    parent::test();
    return $this;
  }
}
