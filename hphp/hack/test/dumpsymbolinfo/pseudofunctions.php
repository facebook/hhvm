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

  isset($c);
  unset($c);

  assert(true);
  invariant(true, 'foo');
  invariant_violation('foo');
}
