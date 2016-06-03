<?hh

class C {
  const int foo = 3;
  public static ?string $foo;
  public static function foo() {}

  public function test() {
    self::$foo;
    $this::$foo;
    C::$foo;
    static::$foo;
  }
}

class D extends C {
  public function test() {
    self::$foo;
    $this::$foo;
    C::$foo;
    static::$foo;
    parent::$foo;
  }
}

type E = C;
newtype F = C;

function test(C $c, D $d, E $e, F $f) {
  $c::$foo;
  $d::$foo;
  $e::$foo;
  $f::$foo;
  C::$foo;
  D::$foo;

  $c::foo();
  C::foo;
}
