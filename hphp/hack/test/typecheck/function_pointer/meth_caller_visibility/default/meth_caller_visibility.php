<?hh

class MyChild {
  public function quux(): void {}
}

class Foo extends MyChild {
  public function test(): void {
    $x = meth_caller(MyChild::class, 'quux');
  }
}

class DifferentClass {
  public function test(): void {
    $x = meth_caller(Foo::class, 'quux');
  }
}
