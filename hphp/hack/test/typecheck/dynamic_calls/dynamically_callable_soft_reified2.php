<?hh

class Foo {

  <<__DynamicallyCallable>>
  public function test<<<__Soft>> reify T>(): void {}
}
