<?hh

class C {
  public static function staticFoo(): void {}
  public function instanceFoo(): void {}
}

function test(C $c, dict<int, int> $d): void {
  echo('foo');
  print('foo');

  tuple(1, 2);
  shape('x' => 5);

  isset($c);
  unset($d[0]);

  invariant(true, 'foo');
  invariant_violation('foo');
}
