<?hh

abstract class A {
  public static function m1(): void {
    static::absolute();
    //          ^ at-caret
  }

  public static abstract function absolute(): void;
}
