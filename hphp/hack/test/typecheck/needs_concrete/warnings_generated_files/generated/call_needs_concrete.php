<?hh

abstract class GeneratedCallNeedsConcrete {
  <<__NeedsConcrete>>
  public static function needs_concrete_method(): void {}

  public static function caller(): void {
    static::needs_concrete_method();
  }
}
