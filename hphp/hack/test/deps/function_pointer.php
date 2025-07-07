<?hh

class A {}

class C {
  const type TC = A;
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


abstract class B {
  abstract const type TC1 as C;
  const type TC2 = arraykey;

  public function buz(this::TC1 $_, this::TC1::TC $_ , this::TC2 $_): void {}

  public static function baz(this $_, this::TC1 $_, this::TC1::TC $_ , this::TC2 $_): void {}
}


function refBuz(): void {
  $_ = meth_caller(B::class, 'buz');
}


function refBaz(): void {
  $_ = B::baz<>;
}
