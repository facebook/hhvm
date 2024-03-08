<?hh

final class Foo {
  public function bar(): void {}
}

function test(): void {
  meth_caller(Foo::class, 'bar');

  // Should error
  meth_caller(NotFound::class, 'bar');
}
