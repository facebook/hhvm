<?hh

class Foo {
  private static $cache = dict[];
  const DELIM = ':';

  public function __construct(string $type, ?string $annot) {
  }

  public static function getCache() :mixed{
    return self::$cache;
  }

  public static function create(string $type, ?string $annot) :mixed{
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

function main() :mixed{
  var_dump(Foo::getCache());
  $y = Foo::create('a', null);
  $y = Foo::create('a', null);
  $y = Foo::getCache();
  var_dump($y);
}

<<__EntryPoint>>
function main_static_props_014() :mixed{
main();
}
