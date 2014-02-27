<?hh

class Foo {
  private static $cache = array();
  const DELIM = ':';

  public function __construct(string $type, ?string $annot) {
  }

  public static function getCache() {
    return self::$cache;
  }

  public static function create(string $type, ?string $annot) {
    if ($annot === null) {
      $key_string = $type;
    } else {
      $key_string = $type.self::DELIM.$annot;
    }

    $key = isset(self::$cache[$key_string]) ? self::$cache[$key_string] : null;
    if ($key === null) {
      $key = new static($type, $annot);
      self::$cache[$key_string] = $key;
    }
    return $key;
  }
}

function main() {
  var_dump(Foo::getCache());
  $y = Foo::create('a', null);
  $y = Foo::create('a', null);
  $y = Foo::getCache();
  var_dump($y);
}
main();
