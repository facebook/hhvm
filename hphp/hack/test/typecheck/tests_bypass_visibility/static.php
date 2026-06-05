<?hh

class WWWTest {}

class StaticFoo {
  <<__TestsBypassVisibility>>
  private static function spriv(): void {}

  <<__TestsBypassVisibility>>
  protected static function sprot(): void {}
}

class StaticTest extends WWWTest {
  public function test(): void {
    StaticFoo::spriv(); // ok
    StaticFoo::sprot(); // ok
  }
}

class StaticNonTest {
  public function test(): void {
    StaticFoo::spriv(); // error
    StaticFoo::sprot(); // error
  }
}
