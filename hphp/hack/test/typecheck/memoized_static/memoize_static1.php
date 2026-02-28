<?hh
class NotFinal {
  <<__Memoize>>
  public static function get(): this {
    return new static();
  }
}
