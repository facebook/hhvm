<?hh

class Base {
  public static function foo(): void {}
}

class Child extends Base {
  <<__NeedsConcrete>>
  public static function foo(): void {}
}
