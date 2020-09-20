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

function takesString(string $s): void {}

function testFakeMembers(C $c, D $d, E $e, F $f): void {
  if ($c::$foo === null) {
    $c::$foo = 'foo';
    takesString($c::$foo);
  }
  if ($d::$foo === null) {
    $d::$foo = 'foo';
    takesString($d::$foo);
  }
  if ($e::$foo === null) {
    $e::$foo = 'foo';
    takesString($e::$foo);
  }
  if ($f::$foo === null) {
    $f::$foo = 'foo';
    takesString($f::$foo);
  }
  if (C::$foo === null) {
    C::$foo = 'foo';
    takesString(C::$foo);
  }
  if (D::$foo === null) {
    D::$foo = 'foo';
    takesString(D::$foo);
  }
}
