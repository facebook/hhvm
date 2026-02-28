<?hh

abstract class A {
  <<__NeedsConcrete>>
  public static function m1(): void {
    static::m2();
    //       ^ at-caret
  }

  public static abstract function m2(): void;
}
