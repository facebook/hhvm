<?hh

class B {
  public static function go() {
    $x = static::make("foo");
    $x->heh();
    return $x;
  }

  public static function make() { throw new Exception('a'); }
}

class C {
  function __construct(private string $x) {}
  function heh() { echo $this->x; }
}

class D1 extends B {
  static function make(string $x) {
    return new C($x);
  }
}

class D2 extends B {
  static function make(string $x) {
    return new C($x);
  }
}

D1::go();
D2::go();
