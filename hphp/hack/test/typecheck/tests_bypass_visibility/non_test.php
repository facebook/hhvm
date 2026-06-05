<?hh

class WWWTest {}

class Foo2 {
  <<__TestsBypassVisibility>>
  private function priv(): void {}

  <<__TestsBypassVisibility>>
  protected function prot(): void {}
}

class NotATest {
  public function test(Foo2 $f): void {
    $f->priv(); // error: not in test context
    $f->prot(); // error: not in test context
  }
}
