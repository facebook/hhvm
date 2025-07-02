<?hh

class C {
  public static function foo<T>(T $_): void {}
  public function bar<T>(T $_): void {}
}

function qux<T>(T $_): void {}

function refFoo() : void {
  $_ =  C::foo<>;
}

function refBar() : void {
  $_ = meth_caller(C::class, 'bar');
}

function refQux() : void {
  $_ =  qux<>;
}
