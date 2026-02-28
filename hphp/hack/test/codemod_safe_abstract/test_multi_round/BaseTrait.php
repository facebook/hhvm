<?hh

trait BaseTrait implements BaseInterface {
  public static abstract function m(): void;

  public static function baseMethod(): void {
    static::m();
  }
}
