<?hh

class Foo {

  <<__DynamicallyCallable>>
  public function test<reify T>(): void {}
}
