<?hh

class Foo {}

<<__EntryPoint>>
function test(): void {
  $y = meth_caller(Foo::class, 'bar$');
}
