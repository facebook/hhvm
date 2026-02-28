<?hh

final abstract class X {
  public static function get(
    int $error_code,
  ): keyset {
    $tags = keyset[];

    $tags = self::add($tags, $error_code, "foo");
    $tags = self::add($tags, $error_code, "bar");
    return $tags;
  }

  private static function add(
    keyset $tags,
    int $error_code,
    string $s,
  ): keyset {
    if ($error_code) {
      $tags[] = $s;
    }
    return $tags;
  }
}

<<__EntryPoint>>
function main() :mixed{
  var_dump(X::get(42));
}
