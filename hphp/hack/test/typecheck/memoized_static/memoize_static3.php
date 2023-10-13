<?hh
class NotFinal {
  <<__Memoize>>
  public static function get(): string {
    return "hello";
  }
}
