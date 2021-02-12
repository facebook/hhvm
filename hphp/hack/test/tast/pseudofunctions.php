<?hh // partial

class C {
  public static function staticFoo(): void {}
  public function instanceFoo(): void {}
}

function test(C $c): void {
  echo('foo');
  print('foo');

  fun('test');
  class_meth(C::class, 'staticFoo');
  inst_meth($c, 'instanceFoo');
  meth_caller(C::class, 'instanceFoo');

  isset($c);
  unset($c);

  invariant(true, 'foo');
  invariant_violation('foo');
}

<<__Pure>>
function rx_test(): void {
  $c = \HH\Rx\mutable(new C());
  $c1 = \HH\Rx\freeze($c);
}
