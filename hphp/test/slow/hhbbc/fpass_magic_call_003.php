<?hh

class Bar {
  static function someUniqueName(&$x) {
  }
}

function blah() { return 'asd'; }
class A {
  private $foo;
  function go() {
    $foo = blah();
    static::someUniqueName($foo);
    var_dump($foo);
    $foo = 2;
    var_dump($foo);
  }
}

class E extends A {
  static public function __callStatic(string $x, array $args) {
    echo "called\n";
  }
}
class F extends A {}

E::go();
