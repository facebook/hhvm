<?hh

class C {
  public static function staticFoo(): void {}
  public function instanceFoo(): void {}
}

function test(C $c): void {
  echo('foo');
  print('foo');

  assert(true);
  invariant(true, 'foo');
  invariant_violation('foo');

  fun('test');
  class_meth(C::class, 'staticFoo');
  inst_meth($c, 'instanceFoo');
  meth_caller(C::class, 'instanceFoo');

  empty($c);
  isset($c);
  unset($c);
}

<<__Rx>>
function rx_test(): void {
  $c = \HH\Rx\mutable(new C());
  \HH\Rx\freeze($c);
}
