<?hh

function heh($x) :mixed{ echo $x . "\n"; }

class C {
  public static function x($string) :mixed{
    foreach (self::$static_arr as $pattern => $result) {
      $pattern = 'wat' . $pattern . 'asd';
      heh($pattern);
    }
    return $string;
  }

  private static $static_arr = dict[
    'a' => 'b',
    'c' => 'd',
    'e' => 'f',
    'g' => 'h',
  ];
}

function main() :mixed{
  var_dump(C::x("foobarkjh1k2j3nn"));
}

<<__EntryPoint>>
function main_iter_007() :mixed{
main();
}
