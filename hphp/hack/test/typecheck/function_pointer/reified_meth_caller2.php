<?hh

interface MyClass {
  public function foo<reify T>(): void;
}

function test(): void {
  meth_caller(MyClass::class, 'foo');
}
