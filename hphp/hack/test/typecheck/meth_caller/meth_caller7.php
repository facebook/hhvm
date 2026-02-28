<?hh

class C {
  public function foo2(int $x, string $y): void {}
}

function test(C $c): void {
  $caller = meth_caller(C::class, 'foo2');
  $caller($c, '', 0);
}
