<?hh

class WWWTest {}

class Base {
  <<__TestsBypassVisibility>>
  private function priv(): void {}
}

class Child extends Base {}

class GrandChild extends Child {}

class SubclassTest extends WWWTest {
  public function test(): void {
    (new Base())->priv(); // ok
    (new Child())->priv(); // ok: private method folded into subclass
    (new GrandChild())->priv(); // ok: private method folded through descendants
  }
}
