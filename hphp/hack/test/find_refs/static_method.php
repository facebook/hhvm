<?hh

class C {
  public static function foo() {
    self::foo();
    C::foo();
  }
}

class D extends C {
  public function f() {
    self::foo();
    static::foo();
  }
}

type E = D;

function test() {
  C::foo();
  D::foo();
  E::foo(); // TODO: This is not detected

  class_meth(C::class, 'foo');
  class_meth(D::class, 'foo');
  class_meth(E::class, 'foo'); // TODO: This is not detected
}
