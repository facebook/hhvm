<?hh

class MyChild {
  protected function quux(): void {}
}

class Foo extends MyChild {
  protected function bar(): void {}

  private function baz(): void {}
}


class DifferentClass {
  public function test(): void {
    $x = meth_caller(Foo::class, 'quux');
    $y = meth_caller(Foo::class, 'bar');
    $z = meth_caller(Foo::class, 'baz');
  }
}
