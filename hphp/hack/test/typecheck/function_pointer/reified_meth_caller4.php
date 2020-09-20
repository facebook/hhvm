<?hh

class MyParent {
  public function foo<reify T>(): void {}
}

class MyClass extends MyParent {}

function test(): void {
  meth_caller(MyClass::class, 'foo');
}
