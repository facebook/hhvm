<?hh
// Test that needs_concrete_override_check is independent of needs_concrete
// Even though needs_concrete might be enabled elsewhere, with
// needs_concrete_override_check=false, no override warning should be emitted

class Base2 {
  public function bar(): void {}
}

class Child2 extends Base2 {
  <<__NeedsConcrete>>
  public function bar(): void {}
}
