<?hh

final class X {
  public static function getVal($val) {
    $ym = self::getArr($val);
    return self::get($ym[0]);
  }

  private static function check($val) {
    if (!is_int($val)) throw new Exception;
  }

  private static function get($a) {
    return $a;
  }

  private static function getArr($val) {
    self::check($val);
    return varray[
      (int)$val
    ];
  }
}
<<__EntryPoint>>
function main_unreachable_inlining() {
;

for ($i = 0; $i < 10; $i++) {
  try {
    var_dump(X::getVal("42"));
  } catch (Exception $e) {
  }
}
var_dump('done');
}
