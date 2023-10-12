<?hh // strict

class C {
  public function var_args(int ...$x): void {}
}

function test(C $c): void {
  $caller = meth_caller(C::class, 'var_args');
  $caller($c);
  $caller($c, 1);
  $args = Vector { 1, 2, 3 };
  $caller($c, ...$args);
}
