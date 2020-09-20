<?hh

final class Foo {
  public function bar(): void {}
}

function test(): void {
  meth_caller(Foo::class, 'bar');

  meth_caller('Foo', 'bar');

  // Should error
  meth_caller('NotFound', 'bar');
}
