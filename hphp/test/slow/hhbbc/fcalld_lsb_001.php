<?hh

class someJunk { public function __construct(private $blah)[] {} }

class Foo {
  private static $foo = vec[];
  protected static function createInstance() :mixed{ return new stdClass; }
  public static function get($x) :mixed{
    self::$foo[] = new someJunk(static::createInstance($x));
  }
}

class Bar extends Foo {
  protected static function createInstance() :mixed{ return new Bar(); }
}

class Baz extends Foo {
  private function __construct()[] {}
  protected static function createInstance() :mixed{ return new Baz(); }
}


<<__EntryPoint>>
function main_fcalld_lsb_001() :mixed{
var_dump(Bar::get(1));
var_dump(Bar::get(1));
var_dump(Baz::get(2));
}
