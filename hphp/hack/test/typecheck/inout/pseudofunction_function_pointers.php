<?hh

function foobar(inout int $x): void {
  $y = foobar<>;
  call_user_func(inout $y, inout $x);
  fun(inout 'foobar');
  $z = 'foo';
  $w = 'bar';
  inst_meth(inout $z, inout $w);
  inst_meth($z, inout $w);
  inst_meth(inout $z, $w);
  meth_caller(inout $z, inout $w);
  meth_caller($z, inout $w);
  meth_caller(inout $z, $w);
  class_meth(inout $z, inout $w);
  class_meth($z, inout $w);
  class_meth(inout $z, $w);
}
