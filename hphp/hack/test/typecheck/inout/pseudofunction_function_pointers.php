<?hh

function foobar(inout int $x): void {
  $y = foobar<>;
  call_user_func(inout $y, inout $x);
  $z = 'foo';
  $w = 'bar';
  meth_caller(inout $z, inout $w);
  meth_caller($z, inout $w);
  meth_caller(inout $z, $w);
}
