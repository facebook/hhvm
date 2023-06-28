<?hh

class C {
  private static $x = dict[];

  public static function test($c, string $n) :mixed{
    if ($n) {
      self::$x[$c] .= 'a'.$n.'b';
    } else {
      self::$x[$c] = '';
    }
  }

  public static function get() :mixed{
    return self::$x;
  }
}

<<__EntryPoint>>
function main() :mixed{
  C::test('k0', '');
  C::test('k0', 'v0');
  C::test('k1', '');
  C::test('k1', 'v1');
  print(json_encode(C::get())."\n");
}
