<?hh

class C {
  private static $a = darray[];
  <<__NEVER_INLINE>>
  public static function setElemLoop($things) {
    self::$a = darray[];
    foreach ($things as $k => $v) {
      self::$a[$k] = $v;
    }
  }
}

<<__EntryPoint>>
function main() {
  C::setElemLoop(darray['a' => dict[], 'b' => 5]);
  echo "Success\n";
}
