<?hh

function foobar(inout int $x): void {
  $z = 'foo';
  $w = 'bar';
  meth_caller(inout $z, inout $w);
  meth_caller($z, inout $w);
  meth_caller(inout $z, $w);
}
