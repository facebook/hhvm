<?hh

class MyParent {
  public function foo<reify T>(): void {}
}

class MyClass extends MyParent {}

function test(): void {
  $x = new MyClass();
  inst_meth($x, 'foo');
}
