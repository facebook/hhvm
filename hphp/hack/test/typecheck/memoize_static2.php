<?hh // strict
/* HH_IGNORE_ERROR[4054]: Ignore the not implementing IMemoizeParam error */
final class Final implements IMemoizeParam {
  <<__Memoize>>
  public static function get(): this {
    return new static();
  }
}
