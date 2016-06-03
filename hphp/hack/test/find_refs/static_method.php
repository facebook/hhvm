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

function test() {
  C::foo();
  D::foo();
}
