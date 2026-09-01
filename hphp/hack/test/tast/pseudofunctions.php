<?hh

class C {
  public static function staticFoo(): void {}
  public function instanceFoo(): void {}
}

function test(C $c, dict<int, int> $d): void {
  echo('foo');
  print('foo');

  test<>;
  C::staticFoo<>;
  meth_caller(C::class, 'instanceFoo');

  isset($c);
  unset($d[0]);

  invariant(true, 'foo');
  invariant_violation('foo');
}
