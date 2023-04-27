<?hh

class C {
  public function instanceFoo(): void {}
}

function test(C $c): void {
  meth_caller(C::class, 'instanceFoo');
  meth_caller(C::class, 'instanceFoo')($c);
}
