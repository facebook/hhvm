<?hh

class C {
  public static function staticFoo(): void {}
  public function instanceFoo(): void {}
}

function test(C $c): void {
  echo('foo');
  print('foo');

  tuple(1, 2);
  shape('x' => 5);

  fun('test');
  class_meth(C::class, 'staticFoo');
  inst_meth($c, 'instanceFoo');
  meth_caller(C::class, 'instanceFoo');

  fun('test')($c);
  class_meth(C::class, 'staticFoo')();
  inst_meth($c, 'instanceFoo')();
  meth_caller(C::class, 'instanceFoo')($c);

  empty($c);
  isset($c);
  unset($c);

  assert(true);
  invariant(true, 'foo');
  invariant_violation('foo');
}
