<?hh

class C {
  public static function staticFoo(): void {}
  public function instanceFoo(): void {}
}

function test(C $c): void {
  fun('test');
  class_meth(C::class, 'staticFoo');
  inst_meth($c, 'instanceFoo');
  meth_caller(C::class, 'instanceFoo');

  fun('test')($c);
  class_meth(C::class, 'staticFoo')();
  inst_meth($c, 'instanceFoo')();
  meth_caller(C::class, 'instanceFoo')($c);
}
