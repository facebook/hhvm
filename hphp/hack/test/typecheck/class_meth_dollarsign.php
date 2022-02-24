<?hh

class Foo {}

<<__EntryPoint>>
function test(): void {
  $x = meth_caller('x$x', 'foo');
  $y = meth_caller('Foo', 'bar$');
}
