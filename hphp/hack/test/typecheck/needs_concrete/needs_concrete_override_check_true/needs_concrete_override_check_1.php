<?hh
// Test that __NeedsConcrete methods cannot override non-__NeedsConcrete methods
// when needs_concrete_override_check is enabled

class Base {
  public static function foo(): void {}
}

class Child extends Base {
  <<__NeedsConcrete>>
  public static function foo(): void {}
}
