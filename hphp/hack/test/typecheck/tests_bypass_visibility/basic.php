<?hh

class WWWTest {}
class AlternateWWWTest {}

class Foo {
  <<__TestsBypassVisibility>>
  private function priv(): void {}

  <<__TestsBypassVisibility>>
  protected function prot(): void {}

  <<__TestsBypassVisibility>>
  private static function spriv(): void {}
}

class FooTest extends WWWTest {
  public function test(Foo $f): void {
    $f->priv(); // ok: bypasses private
    $f->prot(); // ok: bypasses protected
    Foo::spriv(); // ok: bypasses static private
  }
}

class AlternateFooTest extends AlternateWWWTest {
  public function test(Foo $f): void {
    $f->priv(); // ok: bypasses private through the second configured ancestor
    $f->prot(); // ok: bypasses protected through the second configured ancestor
    Foo::spriv(); // ok: bypasses static private through the second configured ancestor
  }
}
