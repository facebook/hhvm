<?hh

class someJunk { public function __construct(private $blah) {} }

class Foo {
  private static $foo;
  protected static function createInstance() { return new stdclass; }
  public static function get($x) {
    self::$foo[] = new someJunk(static::createInstance($x));
  }
}

class Bar extends Foo {
  protected static function createInstance() { return new Bar(); }
}

class Baz extends Foo {
  private function __construct() {}
  protected static function createInstance() { return new Baz(); }
}

var_dump(Bar::get(1));
var_dump(Bar::get(1));
var_dump(Baz::get(2));
