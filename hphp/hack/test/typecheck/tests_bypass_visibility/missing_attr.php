<?hh

class WWWTest {}

class MissingAttrTarget {
  private function priv(): void {}

  protected function prot(): void {}

  private static function spriv(): void {}
}

class MissingAttrTest extends WWWTest {
  public function test(MissingAttrTarget $target): void {
    $target->priv(); // error: missing attribute
    $target->prot(); // error: missing attribute
    MissingAttrTarget::spriv(); // error: missing attribute
  }
}
