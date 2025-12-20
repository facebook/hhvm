<?hh
// Test that __NeedsConcrete methods can override non-__NeedsConcrete methods
// when needs_concrete_override_check is disabled (default)

class Base {
  public function foo(): void {}
}

class Child extends Base {
  <<__NeedsConcrete>>
  public function foo(): void {}
}
