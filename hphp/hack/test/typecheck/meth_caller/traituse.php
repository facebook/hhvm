<?hh

trait MyTrait {
  public function foo(): void {}
}

class MyClass {
  use MyTrait;
}

function test(): void {
  meth_caller(MyClass::class, 'foo');
  meth_caller('MyClass', 'foo');
}
