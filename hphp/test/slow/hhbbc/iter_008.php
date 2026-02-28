<?hh

function heh($x) :mixed{ echo $x . "\n"; }

class C {
  public static function x($string) :mixed{
    foreach (self::$static_arr as $result) {
      $result = 'wat' . $result . 'asd';
      heh($result);
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
function main_iter_008() :mixed{
main();
}
