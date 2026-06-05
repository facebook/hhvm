<?hh

class WWWTest {}

class Parent_ {
  <<__TestsBypassVisibility>>
  protected function prot(): void {}
}

// Overriding a protected method is allowed if the attribute is propagated
class GoodChild extends Parent_ {
  <<__Override, __TestsBypassVisibility>>
  protected function prot(): void {}
}

// Missing the attribute on the override is an error
class BadChild extends Parent_ {
  <<__Override>>
  protected function prot(): void {}
}

class OverrideTest extends WWWTest {
  public function test(Parent_ $obj): void {
    $obj->prot(); // ok: Parent_::prot has the attribute
  }
}
