<?hh

class Foo {
  public function bar(inout int $_): void {}
}


function test(): void {
  $x = HH\meth_caller(Foo::class, 'bar');
}
