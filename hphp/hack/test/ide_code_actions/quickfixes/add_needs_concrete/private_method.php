<?hh

abstract class A {
  private static function m1(): void {
    static::m2();
    //       ^ at-caret
  }

  public static abstract function m2(): void;
}
