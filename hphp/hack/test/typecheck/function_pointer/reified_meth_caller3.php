<?hh

trait MyTrait {
  public function foo<reify T>(): void {}
}

class MyClass {
  use MyTrait;
}

function test(): void {
  meth_caller(MyClass::class, 'foo');
}
