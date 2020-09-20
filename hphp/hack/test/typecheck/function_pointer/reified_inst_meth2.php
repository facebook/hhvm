<?hh

trait MyTrait {
  public function foo<reify T>(): void {}
}

class MyClass {
  use MyTrait;
}

function test(): void {
  $x = new MyClass();
  inst_meth($x, 'foo');
}
