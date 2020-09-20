<?hh

final class MyClass {
  public function foo<reify T>(): void {}
}

function test(): void {
  $x = new MyClass();
  inst_meth($x, 'foo');
}
