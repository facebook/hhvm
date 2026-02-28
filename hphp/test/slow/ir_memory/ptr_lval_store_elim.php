<?hh

class C {
  private static $a = dict[];
  <<__NEVER_INLINE>>
  public static function setElemLoop($things) :mixed{
    self::$a = dict[];
    foreach ($things as $k => $v) {
      self::$a[$k] = $v;
    }
  }
}

<<__EntryPoint>>
function main() :mixed{
  C::setElemLoop(dict['a' => dict[], 'b' => 5]);
  echo "Success\n";
}
