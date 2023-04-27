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

  C::foo<>;
  D::foo<>;
  E::foo<>; // TODO: This is not detected
}
