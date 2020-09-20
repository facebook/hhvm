<?hh

class A {
  public static int $depth = 0;

  <<__MemoizeLSB>>
  public static function get(): string {
    $ret = "@" . self::$depth;
    self::$depth++;
    if (self::$depth < 3) {
      var_dump(static::get());
    }
    self::$depth--;
    return $ret;
  }
}

<<__EntryPoint>> function main(): void {
var_dump(A::get());
var_dump(A::get());
}
