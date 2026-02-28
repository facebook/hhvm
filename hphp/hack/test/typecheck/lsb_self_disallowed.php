<?hh

class C {
  <<__LSB>> private static int $id = 0;

  public static function foo(): int {
    return self::$id;
  }
}
