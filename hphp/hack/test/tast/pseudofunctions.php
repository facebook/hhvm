<?hh

class C {
  public static function staticFoo(): void {}
  public function instanceFoo(): void {}
}

function test(C $c): void {
  echo('foo');
  print('foo');

  test<>;
  C::staticFoo<>;
  meth_caller(C::class, 'instanceFoo');

  isset($c);
  unset($c);

  invariant(true, 'foo');
  invariant_violation('foo');
}
